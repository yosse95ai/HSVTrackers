#include <iostream>
#include <opencv2/opencv.hpp>
#include <OpenNI.h>
#include <time.h>

class MouseEvent
{
public:
	// マウス位置を更新
	void change(int x, int y, int eventType)
	{
		_x = x;
		_y = y;
		_eventType = eventType;
	}
	// マウスイベントのタイプを取得する
	int getEventType()
	{
		return _eventType;
	}
	// マウスのx座標
	int getX()
	{
		return _x;
	}
	// マウスのy座標
	int getY()
	{
		return _y;
	}

private:
	int _x, _y;		// マウス座標( x, y )
	int _eventType; // マウスイベントのタイプ
};

class DepthSensor
{
public:
	void initialize()
	{
		// デバイスを取得
		openni::Status ret = device.open(openni::ANY_DEVICE);
		if (ret != openni::STATUS_OK)
		{
			throw std::runtime_error("openni::Device::open() failed.");
		}

		// カラーストリームを有効にする
		colorStream.create(device, openni::SENSOR_COLOR);
		//cangeResolution(colorStream);
		colorStream.start();
	}

	// フレームの更新処理
	cv::Mat update()
	{
		openni::VideoFrameRef colorFrame;

		// 更新されたフレームの取得
		colorStream.readFrame(&colorFrame);

		// フレームデータを表示できる形に変換する
		frame = showColorStream(colorFrame);

		// フレームデータを表示する
		return frame;
	}

private:
	void changeResolutioin(openni::VideoStream &stream)
	{
		openni::VideoMode mode = stream.getVideoMode();
		mode.setResolution(640, 480);
		mode.setFps(30);
		stream.setVideoMode(mode);
	}

	// カラーストリームを表示できる形に変換する
	cv::Mat showColorStream(const openni::VideoFrameRef &colorFrame)
	{
		// OpeCVの形にする
		cv::Mat frame = cv::Mat(colorFrame.getHeight(), colorFrame.getWidth(),
								CV_8UC3, (unsigned char *)colorFrame.getData());

		// BGR 2 RGB
		cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);

		return frame;
	}

	openni::Device device;			 // 使用するデバイス
	openni::VideoStream colorStream; // カラーストリーム
	cv::Mat frame;					 // 表示用データ
};

// コールバック関数
void CallBackFunction(int eventType, int x, int y, int flags, void *userdata)
{
	MouseEvent *m = static_cast<MouseEvent *>(userdata);
	m->change(x, y, eventType);
}

// 数値の桁をそろえてコンソール出力
void DigitAligment(int val, bool endl = false, bool four = false)
{
	if (!endl)
	{
		if (!four)
		{
			std::cout << std::setw(3) << std::setfill(' ') << val << std::endl;
		}
		else
		{
			std::cout << std::setw(4) << std::setfill(' ') << val << std::endl;
		}
	}
	else
	{
		std::cout << std::setw(3) << std::setfill(' ') << val;
	}
}

/*************************************************************************************/
/* ここからメイン関数 ****************************************************************/
/*************************************************************************************/
int main(int argc, char *argv[])
{
	cv::String winName = "Intput Image";

	// camera用フレーム
	cv::Mat inputImage;
	cv::Mat hsvImage;

	// マウスイベント
	MouseEvent mEvnt;

	// 左クリックイベントのフラグ
	bool isLeftDown = false;

	try
	{
		// OpenNIを初期化
		openni::OpenNI::initialize();

		// センサーを初期化する
		DepthSensor sensor;
		sensor.initialize();
		// メインループ
		while (true)
		{
			inputImage = sensor.update();
			cv::cvtColor(inputImage, hsvImage, cv::COLOR_BGR2HSV);
			cv::imshow(winName, inputImage);
			cv::imshow("HSV Imgae", hsvImage);
			// マウスコールバックの設定
			cv::setMouseCallback(winName, CallBackFunction, &mEvnt);
			// キー入力を取得
			const int key = cv::waitKey(1);
			// 左クリックがあったら表示
			if (mEvnt.getEventType() == cv::EVENT_LBUTTONDOWN)
			{
				// クリック動作一回に対して出力する
				if (!isLeftDown)
				{
					int x = mEvnt.getX(), y = mEvnt.getY();					  // マウスのxy座標取得
					int ocx = y, ocy = x;									  // OpenCV用に座標変換
					int blue = inputImage.at<cv::Vec3b>(ocx, ocy)[0];		  // 指定座標の青色値(0~255)
					int green = inputImage.at<cv::Vec3b>(ocx, ocy)[1];		  // 指定座標の緑色値(0~255)
					int red = inputImage.at<cv::Vec3b>(ocx, ocy)[2];		  // 指定座標の赤色値(0~255)
					int hue = hsvImage.at<cv::Vec3b>(ocx, ocy).val[0];		  // 指定座標の色相(0~360)
					int saturation = hsvImage.at<cv::Vec3b>(ocx, ocy).val[1]; // 指定座標の彩度(0~255)
					int value = hsvImage.at<cv::Vec3b>(ocx, ocy).val[2];	  // 指定座標の明度(0~255)

					// クリック後のマウスの座標を出力
					std::cout << "Pixel : [ ";
					DigitAligment(x, 1, 1);
					std::cout << ", ";
					DigitAligment(y, 1, 1);
					std::cout << " ]" << std::endl;
					// 座標の画素値(R,G,B)を表示
					std::cout << "RGB-----------------" << std::endl;
					std::cout << " R   : ";
					DigitAligment(red);
					std::cout << " G   : ";
					DigitAligment(green);
					std::cout << " B   : ";
					DigitAligment(blue);
					// 座標の画素値(H,S,V)を表示
					std::cout << "HSV-----------------" << std::endl;
					std::cout << " H   : ";
					DigitAligment(hue);
					std::cout << " S   : ";
					DigitAligment(saturation);
					std::cout << " V   : ";
					DigitAligment(value);
					std::cout << "--------------------" << std::endl
							  << std::endl;

					isLeftDown = true;
				}
			}
			// qキーが押されたら終了
			else if (key == 'q')
			{
				cv::destroyAllWindows();
				std::cout << "  Please Press [Enter] Key" << std::endl
						  << std::endl;
				std::getchar();
				break;
			}
			// 何もなければ左クリックイベントのフラグを初期化
			else
			{
				isLeftDown = false;
			}
		}
	}
	catch (std::exception &)
	{
		std::cout << openni::OpenNI::getExtendedError() << std::endl;
	}

	return 0;
}