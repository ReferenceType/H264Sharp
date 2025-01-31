using NAudio.Wave;
using OpenCvSharp;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using OpenH264Sample;
using System.IO;
using System.Diagnostics;
using H264Sharp;
using System.Collections.Concurrent;

namespace AVRecord
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow :System.Windows.Window
    {
        bool stopCam;
        bool rec;
        Mat m = new Mat();
        private WaveInEvent waveIn;
        private H264Encoder encoder;
        private H264Decoder decoder;
        private Stream s;
        private AviWriter writer;
        const int w = 1920;
        const int h = 1080;
        object mtex = new object();
        int numThreads = 4;
        ConverterConfig config = ConverterConfig.Default;
        public MainWindow()
        {
            Environment.SetEnvironmentVariable("OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS", "0");

            encoder = new H264Encoder();
            var param = encoder.GetDefaultParameters();

            param.iUsageType = EUsageType.CAMERA_VIDEO_REAL_TIME;
            param.iPicWidth = w; 
            param.iPicHeight = h;
            param.iTargetBitrate = 1000000;
            param.iTemporalLayerNum = 1;
            param.iSpatialLayerNum = 1;
            param.iRCMode = RC_MODES.RC_BITRATE_MODE;

            param.sSpatialLayers[0].iVideoWidth = 0;
            param.sSpatialLayers[0].iVideoWidth = 0;
            param.sSpatialLayers[0].fFrameRate = 60;
            param.sSpatialLayers[0].iSpatialBitrate = 1000000;
            param.sSpatialLayers[0].uiProfileIdc = EProfileIdc.PRO_HIGH;
            param.sSpatialLayers[0].uiLevelIdc = 0;
            param.sSpatialLayers[0].iDLayerQp = 0;
           

            param.iComplexityMode = ECOMPLEXITY_MODE.HIGH_COMPLEXITY;
            param.uiIntraPeriod = 300;
            param.iNumRefFrame = 0;
            param.eSpsPpsIdStrategy = EParameterSetStrategy.SPS_LISTING_AND_PPS_INCREASING;
            param.bPrefixNalAddingCtrl = false;
            param.bEnableSSEI = true;
            param.bSimulcastAVC = false;
            param.iPaddingFlag = 0;
            param.iEntropyCodingModeFlag = 1;
            param.bEnableFrameSkip = false;
            param.iMaxBitrate =0;
            param.iMinQp = 0;
            param.iMaxQp = 51;
            param.uiMaxNalSize = 0;
            param.bEnableLongTermReference = true;
            param.iLTRRefNum = 1;
            param.iLtrMarkPeriod = 180;
            param.iMultipleThreadIdc = 1;
            param.bUseLoadBalancing = true;
            
            param.bEnableDenoise = false;
            param.bEnableBackgroundDetection = true;
            param.bEnableAdaptiveQuant = true;
            param.bEnableSceneChangeDetect = true;
            param.bIsLosslessLink = false;
            param.bFixRCOverShoot = true;
            param.iIdrBitrateRatio = 400;
            param.fMaxFrameRate = 30;

            //encoder.SetOption(ENCODER_OPTION.ENCODER_OPTION_TRACE_LEVEL, TRACE_LEVEL.WELS_LOG_QUIET);


            encoder.Initialize(param);


            //encoder.Initialize(w, h, 2_000_000, 30, ConfigType.CameraCaptureAdvanced);
            //encoder.SetOption(ENCODER_OPTION.ENCODER_OPTION_LTR, 1);
            //encoder.SetOption(ENCODER_OPTION.ENCODER_LTR_MARKING_PERIOD, 30);

            decoder = new H264Decoder();
            TagSVCDecodingParam decParam = new TagSVCDecodingParam();
            decParam.uiTargetDqLayer = 0xff;
            decParam.eEcActiveIdc = ERROR_CON_IDC.ERROR_CON_FRAME_COPY_CROSS_IDR;
            decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_TYPE.VIDEO_BITSTREAM_SVC;
            decoder.Initialize(decParam);

            config.EnableAvx2 = 1;
            config.NumthreadsRgb2Yuv = 1;
            config.NumthreadsYuv2Rgb = 1;
            Converter.SetConfig(config);
            Cv2.SetNumThreads(1);

            InitializeComponent();

        }

        private void Capture(object sender, RoutedEventArgs e)
        {
            stopCam = false;
            CaptureCam();
            CaptureAudio();
        }
        private void BeginRecord(object sender, RoutedEventArgs e)
        {
            /*
             * this recoder has issues with audio and image sync.
             * i will rewrite it at a later time
             */
            lock (mtex)
            {
                if (rec)
                {
                    writer.Close();
                    //s.Close();
                }
                else
                {

                    encoder.ForceIntraFrame();
                    if (File.Exists("Vid.avi"))
                        File.Delete("Vid.avi");
                    s = new FileStream("Vid.avi", FileMode.OpenOrCreate, FileAccess.Write);
                    writer = new AviWriter(s, "H264", w, h, 30);
                }
                rec = !rec;
            }

        }
        private void Stop(object sender, RoutedEventArgs e)
        {
            stopCam = true;
            StopAudio();
            //stop avi
        }


        private void CaptureCam()
        {
            var capture = new VideoCapture(0, VideoCaptureAPIs.WINRT);
          
            capture.Open(0);
            capture.FrameWidth = w;
            capture.FrameHeight = h;
            capture.Fps = 30;
            Mat frame = new Mat();
            Thread t =  new Thread(() =>
            {
                while (!stopCam)
                {
                    if (capture.Read(frame))
                        MatAvailable(frame);
                    //Thread.Sleep(1);
                }

                capture.Release();
                capture.Dispose();

            });
            t.Start();
        }



        private void CaptureAudio()
        {
            var format = new WaveFormat(24000, 16, 1);
            waveIn = new WaveInEvent();
            waveIn.WaveFormat = format;
            waveIn.BufferMilliseconds = 20;
            waveIn.DataAvailable += MicrophoneSampleAvailable;

            soundListenBuffer = new BufferedWaveProvider(format);


            waveOut = new WaveOutEvent();
            waveOut.Init(soundListenBuffer);
            waveOut.Play();
            waveIn.StartRecording();
        }

        private void StopAudio()
        {
            waveIn?.StopRecording();
            waveIn?.Dispose();
        }

        private void MicrophoneSampleAvailable(object? sender, WaveInEventArgs e)
        {
            lock (mtex)
            {
              
                soundListenBuffer.AddSamples(e.Buffer, 0, e.BytesRecorded);
                if (!rec) return;              
                writer.AddAudio(e.Buffer,0,e.BytesRecorded);
            }

        }

      ConcurrentBag<RgbImage> pool = new ConcurrentBag<RgbImage>();
        private RgbImage GetContainer()
        {
            if (pool.TryTake(out var container))
                return container;
            return new RgbImage(w,h);
        }
        class EncodedData
        {
            public DateTime time;
            public byte[] data;
            public int len;
            public bool isKey;
        }
        int ctr = 0;
        double tot = 0;
        double avg = 0;
        double perSecCtr = 0;
        int byteCnt = 0;
        int frameCnt = 0;

        bool enableJitter = false;
        bool enablecv = false;
        private bool enableloss = false;
        Stopwatch sw = Stopwatch.StartNew();
        SLTRRecoverRequest recoverRequest = new SLTRRecoverRequest();
        byte[] dataBuffer = new byte[250000];
        double decodeTime = 0;
        double encodeTime = 0;
        SEncoderStatistics ss;
        SDecoderStatistics ss1;
        private unsafe void MatAvailable(Mat frame)
        {
           encoder.GetOptionRef(ENCODER_OPTION.ENCODER_OPTION_GET_STATISTICS, ref ss);
           decoder.GetOptionRef(DECODER_OPTION.DECODER_OPTION_GET_STATISTICS, ref ss1);
            lock (mtex)
            {
                frameCnt++;
                DrawRawImg(frame);
                Stopwatch s = Stopwatch.StartNew();
                bool encodedSuccess = false;
                H264Sharp.EncodedData[] ec;
                
                if (enablecv)
                {
                    var src = InputArray.Create(frame);
                    var @out = OutputArray.Create(m);
                    Cv2.CvtColor(src, @out, ColorConversionCodes.BGR2YUV_I420);
                    encodedSuccess = encoder.Encode(m.DataPointer, out ec);

                }
                else
                {
                    var g = new ImageData(ImageType.Bgr, frame.Width, frame.Height, (int)frame.Step(), new IntPtr(frame.DataPointer));
                    encodedSuccess = encoder.Encode(g, out ec);
                }
                
              
                ctr++;
                if (encodedSuccess)
                {
                    var len = ec.Sum(x => x.Length); ;
                    byteCnt += len;
                    Trace.WriteLine("Bytes : " + len);
                    if (enableloss)
                    {
                        var rn = random.Next(0, 100);
                        if (rn % 5 == 0 && ec[0].FrameType != FrameType.IDR)
                            return;
                    }

                   

                    Trace.WriteLine(ec[0].FrameType);
                    if (enableJitter)
                    {
                        DateTime now = DateTime.Now;
                        var sum = ec.Sum(x => x.Length);
                        byte[] buffer = new byte[sum];

                        H264Sharp.EncodedData.CopyTo(ec, buffer, 0);
                        EncodedData d = new EncodedData()
                        {
                            time = now,
                            data = buffer,
                            len = len,
                            isKey = ec[0].FrameType == FrameType.IDR
                        };

                        Task.Delay(random.Next(0, 200)).ContinueWith(s =>
                        {
                            Decode(d);
                        });
                    }
                    else
                    {
                        DateTime now = DateTime.Now;
                        var sum = ec.Sum(x => x.Length);
                       

                       H264Sharp.EncodedData.CopyTo(ec, dataBuffer, 0);
                        EncodedData d = new EncodedData()
                        {
                            time = now,
                            data = dataBuffer,
                            len=len,
                            isKey = ec[0].FrameType == FrameType.IDR
                        };
                        if (rec)
                        {
                           
                            writer.AddImage(d.data,0,d.len, d.isKey);
                        }

                        Decode(d);

                    }
                }

                perSecCtr = sw.Elapsed.TotalMilliseconds;

                avg = (30 * avg + s.Elapsed.TotalMilliseconds) / 31;



                if (perSecCtr > 1000)
                {
                    sw.Restart();
                    Dispatcher.BeginInvoke(new Action(() =>
                    {
                        AvgEncoderTime.Text = "Avg transcode time : " + avg.ToString("N3");
                        Fps.Text = $"{frameCnt} fps";
                        DataRate.Text = $"{byteCnt / 1024} Kb/sec";
                        perSecCtr = 0;
                        byteCnt = 0;
                        frameCnt = 0;
                    }));

                }


                //Trace.WriteLine(s.Elapsed.TotalMilliseconds);
                //Trace.WriteLine("AVG  " + avg);
            }

        }
        DateTime lastTime = DateTime.Now;
        private object l = new object();
        Random random = new Random();
        private void Decode(EncodedData data)
        {

            lock (l)
            {
                var time = data.time;
                var buffer = data.data;
                if (time < lastTime && !data.isKey)
                {
                    lastTime = time;
                    Trace.WriteLine("Skip");
                    return;
                }
                lastTime = time;

              

                var rgb = GetContainer();
                bool decodeSucess = decoder.Decode(buffer, 0, data.len, false, out var ds, ref rgb);

                CheckMarkingFeedback(ds);

                if (decodeSucess)
                {
                    //s.Stop();
                    DrawEncodedImg(rgb);
                    ManageError(ds);

                }
                else if(ds.HasFlag(DecodingState.dsNoParamSets))
                {
                    Trace.WriteLine("FORCE FFFFFFFFFFFFFFFFFFFFFFFFF");

                    Trace.Write(ds);
                    pool.Add(rgb);
                    encoder.ForceIntraFrame();

                }
                //if (ec[0].FrameType == FrameType.IDR || ec[0].FrameType == FrameType.I)
                //    Trace.WriteLine("##############################   KEY FRAME  ###################");
                //Trace.WriteLine(encoded.FrameType);
            }

        }
        private void ManageError(DecodingState ds)
        {
            if (ds == DecodingState.dsErrorFree)
            {
                decoder.GetOption(DECODER_OPTION.DECODER_OPTION_IDR_PIC_ID, out int tempInt);
                if (recoverRequest.uiIDRPicId != tempInt)
                {
                    recoverRequest.uiIDRPicId = (uint)tempInt;
                    recoverRequest.iLastCorrectFrameNum = -1;
                }

                if (decoder.GetOption(DECODER_OPTION.DECODER_OPTION_FRAME_NUM, out tempInt))
                    if (tempInt >= 0)
                    {
                        recoverRequest.iLastCorrectFrameNum = tempInt;
                    }
            }
            else if (!ds.HasFlag(DecodingState.dsNoParamSets))
            {
                var recoverRequest = new SLTRRecoverRequest();
                recoverRequest.uiFeedbackType = (uint)KEY_FRAME_REQUEST_TYPE.LTR_RECOVERY_REQUEST;

                decoder.GetOption(DECODER_OPTION.DECODER_OPTION_FRAME_NUM, out int currFrame);
                recoverRequest.iCurrentFrameNum = currFrame;

                decoder.GetOption(DECODER_OPTION.DECODER_OPTION_IDR_PIC_ID, out uint picId);
                recoverRequest.uiIDRPicId = picId;

                recoverRequest.iLastCorrectFrameNum = this.recoverRequest.iLastCorrectFrameNum;
                //var rn = random.Next(0, 100); // passes, reliability not required
                //if (rn % 5 == 0)
                //    return;

                //Task.Delay(51).ContinueWith((t)=>encoder.SetOption(ENCODER_OPTION.ENCODER_LTR_RECOVERY_REQUEST, recoverRequest));
               encoder.SetOption(ENCODER_OPTION.ENCODER_LTR_RECOVERY_REQUEST, recoverRequest);
                Trace.WriteLine(ds);
                Trace.WriteLine($"rec req. currFrame : {recoverRequest.iCurrentFrameNum}, pic id : {recoverRequest.uiIDRPicId}");

            }
            else//not possible
            {
                Trace.WriteLine("intra req");

                encoder.ForceIntraFrame();
            }
        }

        private void CheckMarkingFeedback(DecodingState ds)
        {
            decoder.GetOption(DECODER_OPTION.DECODER_OPTION_LTR_MARKING_FLAG, out int flag);
            if (flag == 1)
            {
                //mark feedback

                var fb = new SLTRMarkingFeedback();
                fb.uiFeedbackType = (/*ds == DecodingState.dsErrorFree*/true) ?
                      (uint)(KEY_FRAME_REQUEST_TYPE.LTR_MARKING_SUCCESS)
                    : (uint)(KEY_FRAME_REQUEST_TYPE.LTR_MARKING_FAILED);

                decoder.GetOption(DECODER_OPTION.DECODER_OPTION_IDR_PIC_ID, out uint pid);
                fb.uiIDRPicId = pid;
                decoder.GetOption(DECODER_OPTION.DECODER_OPTION_LTR_MARKED_FRAME_NUM, out int fnum);
                fb.iLTRFrameNum = fnum;
                //var rn = random.Next(0, 100); // passed, reliabile link not required.
                //if (rn % 5 == 0)
                //    return;

                //Task.Delay(51).ContinueWith((t) => encoder.SetOption(ENCODER_OPTION.ENCODER_LTR_MARKING_FEEDBACK, fb));
                encoder.SetOption(ENCODER_OPTION.ENCODER_LTR_MARKING_FEEDBACK, fb);
                Trace.WriteLine($"Marking fb. Succ:{fb.uiFeedbackType} - ltr num: {fb.iLTRFrameNum}, pic id: {fb.uiIDRPicId},");

            }
        }

       
        void DrawRawImg(Mat frame)
        {
            //return;
            Dispatcher.BeginInvoke(new Action(() =>
            {
                if (RawImg.Source == null)
                {
                    RawImg.Source = new WriteableBitmap(frame.Width, frame.Height, 96, 96,
                         PixelFormats.Bgr24, null);
                }

                var dst = (WriteableBitmap)RawImg.Source;
                dst.Lock();
                int width = frame.Width;
                int height = frame.Height;
                int step = (int)frame.Step();
                long range = frame.DataEnd.ToInt64() - frame.Data.ToInt64();

                dst.WritePixels(new Int32Rect(0, 0, width, height), frame.Data, (int)range, step);
                dst.Unlock();
            }));

        }
        AutoResetEvent drawn = new AutoResetEvent(false);
        private BufferedWaveProvider soundListenBuffer;
        private WaveOutEvent waveOut;
        void DrawEncodedImg(RgbImage frame)
        {
            //return;
            int w = frame.Width;
            int h = frame.Height;
            int stride = frame.Stride;
            Dispatcher.BeginInvoke(new Action(() =>
            {
                try
                {
                    if (Decoded.Source == null)
                    {
                        Decoded.Source = new WriteableBitmap(w, h, 96, 96,
                             PixelFormats.Bgr24, null);
                    }

                    var dst = (WriteableBitmap)Decoded.Source;
                    dst.Lock();
                    int width = w;
                    int height = h;
                    int step = stride;
                    int range = w * h * 3;

                    dst.WritePixels(new Int32Rect(0, 0, width, height), frame.ImageBytes, range, step);

                    dst.Unlock();
                    pool.Add(frame);

                }
                finally
                {
                    //drawn.Set();
                }

            }));
            //drawn.WaitOne();

        }

        private void Slider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            Trace.WriteLine("Target changed");
            encoder?.SetMaxBitrate((int)e.NewValue * 1000);
        }
        private void Slider_ValueChanged2(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            Trace.WriteLine("Target changed");
            numThreads= (int)e.NewValue;
        }

        private void ParallelConverterChecked(object sender, RoutedEventArgs e)
        {
            if (encoder == null)
                return;

            var t = ((CheckBox)sender).IsChecked ?? false ? numThreads : 0;

            config.NumthreadsRgb2Yuv = t;
            config.NumthreadsYuv2Rgb = t;
            Converter.SetConfig(config);
            Cv2.SetNumThreads(t);

        }
        private void ParallelConverterUnChecked(object sender, RoutedEventArgs e)
        {
            if (encoder == null)
                return;
            var t = ((CheckBox)sender).IsChecked ?? false ? numThreads : 0;
            config.NumthreadsRgb2Yuv = t;
            config.NumthreadsYuv2Rgb = t;
            Converter.SetConfig(config);
            Cv2.SetNumThreads(t);
        }

        private void SSEChecked(object sender, RoutedEventArgs e)
        {
            if (decoder == null)
                return;
           var avx = ((CheckBox)sender).IsChecked ?? false;
            config.EnableAvx2 = avx?1:0;
            Converter.SetConfig(config);

        }
        private void SSEUnChecked(object sender, RoutedEventArgs e)
        {
            if (decoder == null)
                return;
            var avx = ((CheckBox)sender).IsChecked ?? false;
            config.EnableAvx2 = avx ? 1 : 0;
            Converter.SetConfig(config);

        }
      

        private void CVChecked(object sender, RoutedEventArgs e)
        {
           
            enablecv = true;

        }
        private void CVUnChecked(object sender, RoutedEventArgs e)
        {
            enablecv = false;

        }
        private void LossChecked(object sender, RoutedEventArgs e)
        {

            enableloss = true;

        }
        private void LossUnChecked(object sender, RoutedEventArgs e)
        {
            enableloss = false;

        }
        private void JitterChecked(object sender, RoutedEventArgs e)
        {

            enableJitter = true;

        }
        private void JitterUnChecked(object sender, RoutedEventArgs e)
        {
            enableJitter = false;

        }
    }
}