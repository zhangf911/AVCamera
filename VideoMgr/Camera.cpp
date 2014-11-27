
#include "stdafx.h"
#include "Camera.h"
#include "util.h"
#include "H264Writer.h"
#include "Filter.h"

//#include <chrono>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

using namespace System;
using namespace System::Threading;

#using < mscorlib.dll >
#using < System.Windows.Forms.dll >


namespace VideoMgr
{

	Camera::Camera()
		: _status(CREATED)
	{
		ffmpeg_init_all();
	}

	Camera::~Camera()
	{
		delete _video;
	}


	void Camera::thread_task( String^ file, int width, int height, int bit_rate )
	{
		H264Writer h264;
		h264.create(file, width, height, bit_rate);

		_video = new cv::VideoCapture();
		_video->open(0);

		Filter filter(width, height);
		cv::Mat frame;
		DateTime timer, curr_time;
		timer = curr_time = DateTime::Now;
		long duration;
		while (true)
		{
			if( _status == PAUSED ) 
			{
				cv::waitKey(100); continue;
			}
			else if( _status == STOPPED ) break;
			else if( _status == RECORDING )
			{
				*_video >> frame;
				// Filter
				bool isEncode = true;
				isEncode = isEncode && filter.show_datetime(frame) && filter.give_up_frame(frame, 15);

				//
				cv::imshow("video", frame);
				
				curr_time = DateTime::Now;
				
				if(isEncode) 
				{				
					long dur = curr_time.Subtract(timer).Ticks;
					duration += dur;
					h264.write(frame, duration/10000 > 33 ? abs(33 - duration/10000) : 33);
					cv::waitKey(duration/10000 >= 33 ? 33 : abs(33 - duration/10000));
					duration = 0;
				}
				else
				{
					long dur = curr_time.Subtract(timer).Ticks;
					duration += dur;
					cv::waitKey( dur/10000 );
				}
				//String^ ddd = gcnew String("");
				//ddd += duration.Ticks;
				//System::Windows::Forms::MessageBox::Show(ddd);
				
				timer = curr_time;
				
			}
		}
		cv::destroyWindow("video");
		h264.close();
	}


	int Camera::start()
	{
		if(_status == CREATED)
		{
			_status = RECORDING;
		}
		else if(_status == PAUSED)
		{
			_status = RECORDING;
		}
		else if(_status == STOPPED)
		{
			_status = RECORDING;
		}
		return safe_cast<int>(_status);
	}

	int Camera::pause()
	{
		if(!_video->isOpened()) return 0;
		if(_status == RECORDING)
		{
			_status = PAUSED;
		}
		return safe_cast<int>(_status);
	}

	int Camera::stop()
	{
		if(!_video->isOpened()) return 0;
		if(_status == CREATED)
		{
			_status = STOPPED;
		}
		else if(_status == RECORDING)
		{
			_status = STOPPED;
		}
		else if(_status == PAUSED)
		{
			_status = STOPPED;
		}
		return safe_cast<int>(_status);
	}

}