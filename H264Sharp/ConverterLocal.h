#pragma once
#include"pch.h"

	void BGRAtoYUV420Planar(unsigned char* rgba, unsigned char* dst, int width, int height, int stride);
	void BGRtoYUV420Planar(unsigned char* rgba, unsigned char* dst, int width, int height, int stride);
	void RGBAtoYUV420Planar(unsigned char* rgba, unsigned char* dst, int width, int height, int stride);
	void RGBtoYUV420Planar(unsigned char* rgb, unsigned char* dst, int width, int height, int stride);
    void Yuv420P2Rgb(unsigned char* dst_ptr,
        const unsigned char* y_ptr,
        const unsigned char* u_ptr,
        const unsigned char* v_ptr,
        signed   int   width,
        signed   int   height,
        signed   int   y_span,
        signed   int   uv_span,
        signed   int   dst_span,
        signed   int   dither);

