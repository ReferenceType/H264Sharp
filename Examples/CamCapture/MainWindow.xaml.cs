using NAudio.Wave;
using OpenCvSharp;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.IO;
using System.Diagnostics;
using H264Sharp;
using System.Collections.Concurrent;

namespace AVRecord
{
   
    public partial class MainWindow :System.Windows.Window
    {
        bool captureActive;
        bool captureBusy;
        bool rec;
        private WaveInEvent waveIn;
        private H264Encoder encoder;
        private H264Decoder decoder;
        private Stream s;
        const int CamWidth = 1280;
        const int CamHeight = 720;
        object mtex = new object();
        int numThreads = 4;
        public MainWindow()
        {

            InitializeComponent();

        }

        private void InitializeTranscoder(int w, int h)
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
            //param.bEnableSSEI = true;
            param.bSimulcastAVC = false;
            param.iPaddingFlag = 0;
            param.iEntropyCodingModeFlag = 1;
            param.bEnableFrameSkip = false;
            param.iMaxBitrate = 0;
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


            TagSVCDecodingParam decParam = new TagSVCDecodingParam();
            decParam.uiTargetDqLayer = 0xff;
            decParam.eEcActiveIdc = ERROR_CON_IDC.ERROR_CON_FRAME_COPY_CROSS_IDR;
            decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_TYPE.VIDEO_BITSTREAM_SVC;

            decoder = new H264Decoder();
            decoder.Initialize(decParam);

            Converter.SetOption(ConverterOption.NumThreads, Environment.ProcessorCount);
        }

        private void Capture(object sender, RoutedEventArgs e)
        {
            if (captureActive)
            {
                captureActive = false;

                AvgEncoderTime.Text = "";
                Fps.Text = "";
                DataRate.Text = "";

                return;

            }
            if (captureBusy)
                return;


            captureActive = true;
            CaptureCam();
        }


        private void CaptureCam()
        {
            captureBusy = true;
            var capture = new VideoCapture(0, VideoCaptureAPIs.WINRT);
            if (!capture.Open(0))
            {
                MessageBox.Show("Unable to acquire camera");
                return;
            }
            capture.FrameWidth = CamWidth;
            capture.FrameHeight = CamHeight;
            capture.Fps = 30;

            InitializeTranscoder(capture.FrameWidth, capture.FrameHeight);

            Mat frame = new Mat();
            Thread t =  new Thread(() =>
            {
                while (captureActive)
                {
                    if (capture.Read(frame))
                        CameraFrameAvailable(frame);
                }

                capture.Release();
                capture.Dispose();
                
                encoder.Dispose();
                decoder.Dispose();
                captureBusy = false;

            });
            t.Start();
        }


        ConcurrentBag<RgbImage> pool = new ConcurrentBag<RgbImage>();
        private RgbImage GetRGBContainer()
        {
            if (pool.TryTake(out var container))
                return container;
            return new RgbImage(ImageFormat.Rgb,CamWidth,CamHeight);
        }

        class EncodedDataInfo
        {
            public DateTime time;
            public byte[] data;
            public int len;
            public bool isKey;
        }

        int encodedCounter = 0;
        double avg = 0;
        double deltaT = 0;
        int byteCnt = 0;
        int frameCnt = 0;

        bool enableCVConverter = false;

        Stopwatch sw = Stopwatch.StartNew();
        byte[] dataBuffer = new byte[250000];
        Mat m = new Mat();
        double cumulative;
        private bool enableloss = false;

        private unsafe void CameraFrameAvailable(Mat frame)
        {

            lock (mtex)
            {
                frameCnt++;
                DrawOriginalImg(frame);

                bool encodedSuccess = false;
                EncodedData[] ec;
                Stopwatch swLocal = Stopwatch.StartNew();

                if (enableCVConverter)
                {
                    var src = InputArray.Create(frame);
                    var @out = OutputArray.Create(m);
                    Cv2.CvtColor(src, @out, ColorConversionCodes.BGR2YUV_I420);
                    var yuvp= new YUVImagePointer(m.Data, frame.Width, frame.Height);
                    encodedSuccess = encoder.Encode(yuvp, out ec);

                }
                else
                {
                    var g = new RgbImage(ImageFormat.Bgr, frame.Width, frame.Height, (int)frame.Step(), new IntPtr(frame.DataPointer));
                    encodedSuccess = encoder.Encode(g, out ec);
                }
              

                if (encodedSuccess)
                {
                    if (enableloss)
                    {
                        var rn = random.Next(0, 100);
                        if (rn % 5 == 0 && ec[0].FrameType != FrameType.IDR)
                            return;
                    }

                    ManageEncodedFrame(ec);
                }

                swLocal.Stop();
                cumulative += swLocal.Elapsed.TotalMilliseconds;

                deltaT = sw.Elapsed.TotalMilliseconds;

               // avg = (30 * avg + s.Elapsed.TotalMilliseconds) / 31;


                if (deltaT > 1000)
                {
                    avg = cumulative / frameCnt;
                    cumulative = 0;

                    sw.Restart();
                    Dispatcher.BeginInvoke(new Action(() =>
                    {
                        AvgEncoderTime.Text = "Avg transcode time : " + avg.ToString("N3");
                        Fps.Text = $"{frameCnt} fps";
                        DataRate.Text = $"{byteCnt / 1024} Kb/sec";
                        deltaT = 0;
                        byteCnt = 0;
                        frameCnt = 0;
                    }));

                }


                //Trace.WriteLine(s.Elapsed.TotalMilliseconds);
                //Trace.WriteLine("AVG  " + avg);
            }

        }

        bool enableJitter = false;
        private unsafe void ManageEncodedFrame(H264Sharp.EncodedData[] ec)
        {
            var encodedLength = ec.Sum(x => x.Length); ;
            byteCnt += encodedLength;

            Trace.WriteLine("Bytes : " + encodedLength);
            Trace.WriteLine(ec[0].FrameType);

            if (enableJitter)
            {
                byte[] buffer = ec.GetAllBytes();
                EncodedDataInfo d = new EncodedDataInfo()
                {
                    time = DateTime.Now,
                    data = buffer,
                    len = encodedLength,
                    isKey = ec[0].FrameType == FrameType.IDR
                };

                Task.Delay(random.Next(0, 200)).ContinueWith(s =>
                {
                    Decode(d);
                });
            }
            else
            {

                ec.CopyAllTo(dataBuffer, 0);
                EncodedDataInfo d = new EncodedDataInfo()
                {
                    time = DateTime.Now,
                    data = dataBuffer,
                    len = encodedLength,
                    isKey = ec[0].FrameType == FrameType.IDR
                };

                Decode(d);

            }
        }

        DateTime lastTime = DateTime.Now;
        private object l = new object();
        Random random = new Random();
        private void Decode(EncodedDataInfo data)
        {

            lock (l)
            {
                // this is to discard stale frames when jitter mode enabled.
                var time = data.time;
                var buffer = data.data;
                if (time < lastTime && !data.isKey)
                {
                    lastTime = time;
                    Trace.WriteLine("Skip");
                    return;
                }
                lastTime = time;

              
                var rgb = GetRGBContainer();
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
                    Trace.WriteLine("FORCE IDR");

                    Trace.Write(ds);
                    pool.Add(rgb);
                    encoder.ForceIntraFrame();

                }
                //if (ec[0].FrameType == FrameType.IDR || ec[0].FrameType == FrameType.I)
                //    Trace.WriteLine("##############################   KEY FRAME  ###################");
                //Trace.WriteLine(encoded.FrameType);
            }

        }

        //Here you will se Advanced LTR management, which is much more efficient than Foce IDR recoveries.
        // if you dont care about LTR you can just force sender to do IDR on error.
        SLTRRecoverRequest recoverRequest = new SLTRRecoverRequest();

        private void ManageError(DecodingState ds)
        {
            // No error, decoder just need to keep track of last correct frame on each frame.
            if (ds == DecodingState.dsErrorFree)
            {
                decoder.GetOption(DECODER_OPTION.DECODER_OPTION_IDR_PIC_ID, out int IdcPicId);
                if (recoverRequest.uiIDRPicId != IdcPicId)
                {
                    recoverRequest.uiIDRPicId = (uint)IdcPicId;
                    recoverRequest.iLastCorrectFrameNum = -1;
                }

                if (decoder.GetOption(DECODER_OPTION.DECODER_OPTION_FRAME_NUM, out int frameNum))
                {
                    if (frameNum >= 0)
                    {
                        recoverRequest.iLastCorrectFrameNum = frameNum;
                    }
                }
                    
            }
            // an error(recoverable) send a recovery request.
            else if (!ds.HasFlag(DecodingState.dsNoParamSets))
            {
                var recoverRequest = new SLTRRecoverRequest();
                recoverRequest.uiFeedbackType = (uint)KEY_FRAME_REQUEST_TYPE.LTR_RECOVERY_REQUEST;

                decoder.GetOption(DECODER_OPTION.DECODER_OPTION_FRAME_NUM, out int currFrame);
                recoverRequest.iCurrentFrameNum = currFrame;

                decoder.GetOption(DECODER_OPTION.DECODER_OPTION_IDR_PIC_ID, out uint picId);
                recoverRequest.uiIDRPicId = picId;

                //notice why we keep track of lst correct frame above.
                recoverRequest.iLastCorrectFrameNum = this.recoverRequest.iLastCorrectFrameNum;

                // emulate loss/jiter
                //var rn = random.Next(0, 100); // passes, reliability not required
                //if (rn % 5 == 0)
                //    return;

                //Task.Delay(51).ContinueWith((t)=>encoder.SetOption(ENCODER_OPTION.ENCODER_LTR_RECOVERY_REQUEST, recoverRequest));


                // So here sender will handle the recovery request, we just put it here on encoder
                encoder.SetOption(ENCODER_OPTION.ENCODER_LTR_RECOVERY_REQUEST, recoverRequest);
                Trace.WriteLine(ds);
                Trace.WriteLine($"rec req. currFrame : {recoverRequest.iCurrentFrameNum}, pic id : {recoverRequest.uiIDRPicId}");

            }
            else//not possible to recover, we need a fresh start(who doesnt..)
            {
                Trace.WriteLine("intra req");

                // recovery not possible sender must force IDR
                encoder.ForceIntraFrame();
            }
        }

        // receiver sends a marking feedback to sender when the frame is LTR so sender knows which is last. 
        private void CheckMarkingFeedback(DecodingState ds)
        {
            decoder.GetOption(DECODER_OPTION.DECODER_OPTION_LTR_MARKING_FLAG, out int isMarking);
            if (isMarking == 1)
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

                // Receiver sends this marking feedback to sender.  and sender does the following
                encoder.SetOption(ENCODER_OPTION.ENCODER_LTR_MARKING_FEEDBACK, fb);
                Trace.WriteLine($"Marking fb. Succ:{fb.uiFeedbackType} - ltr num: {fb.iLTRFrameNum}, pic id: {fb.uiIDRPicId},");

            }
        }

       
        void DrawOriginalImg(Mat frame)
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

                    dst.WritePixels(new Int32Rect(0, 0, width, height), frame.NativeBytes, range, step);

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

        private void CVChecked(object sender, RoutedEventArgs e)
        {
           
            enableCVConverter = true;

        }
        private void CVUnChecked(object sender, RoutedEventArgs e)
        {
            enableCVConverter = false;

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