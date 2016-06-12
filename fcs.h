#pragma once

#include <cassert>
#include <vector>
#include <iostream>
#include <fstream>
#include <memory>
#include "opencv2\opencv.hpp"

namespace fcs
{
	typedef short TDist; // Тип данных для хранения координат писелей
	typedef float TResp; // Тип данных для хранения отклика фильтра

	struct TFeature // уникальные элементы изображения (недообъекты) 
	{
		enum Type : uint8_t //Тип фильтра который использовали для найденного данного уникального элемента
		{	
		 LoG,	// Функция Лапласиан Гаусиана - плосовой фильтр сигма - полоса
		 DoG,	// Разность гауссиан - тоже полосовой фильтр но с другими частотными характеристиками
		 Gabor,	// фильтры габора. Имеют в хра-ке частоту и направление
		 Hess	// Гессиан
		};  
														// 

		TDist x, y;      // координаты особенности в кадре
		TResp response;  // абсолютный отклик
		TResp majority;  // относительный отлик ( контрастность относительно среднего )
		uint8_t aluID;   // 
		uint8_t rsrvd;   //
		Type  type;      // тип точки
		short size;      // характерный размер точки (диаметр в пикселях)

		TFeature() : majority(0) {}; // Фича должна обязательно создаваться с 0 вым полем в majority. Если majority == 0, то эта фича битая ( неправильная )
	};

	struct TBin {
		char* bin;       // бинарные образы откликов
		int filterSize;  // не используется
	};

	struct TPrimary {    // тип который возвращает первичка
		std::vector<fcs::TFeature> feat; // набор уникальных точек
		std::vector<fcs::TBin> segm;     // набор бинарных образов
	};

	typedef std::vector<cv::Point> TSegm;  // тип для хранения сегмента объекта

	struct object {              // данный тип возвращает вторичка
		enum Part : uint32_t     // какие поля валидны для данного объекта
		{
			Feature = 0x1,  // уникальный элемент объекта       
			Segment = 0x2,  // бинарная маска объекта
			Boundary = 0x4, // граница объекта (набор граничных пикселей)
			Bbox     = 0x8  // обрамляющий прямоугольник вокруг маски 
		};
		int capability; // показывает какие поля актуальны для данного объекта

		TFeature mainFeat; // главная характерная точка
		TSegm segment;     // сегмент (бинарная маска) в абсолютных координатах
		TSegm boundary;    // маска границы
		cv::Rect bbox;     // обрамляющий прямоугольник
		int channelId;     // канал ведения 

		object() : capability(0), channelId(0) {};
	};

	struct Control { // по каждой фичи интереса контролирует состояние фазового автомата в плис
		struct channel{
			enum mode { AutoDetection, Detection, Tracking }; // автодетктирование, ручное наведение, сопровождение
			cv::Rect roi; // у каждого канала своя область инетереса
			mode status;  // и свой режим работы
		};
		std::vector<channel> ch; // каналы ведения объектов. 
		std::vector<cv::Rect> antiRoi;  // запреты на анализ областейю Глобальны по всем каналам
	};

	class IInputProvider { // класс для чтения видеофайлов
	public:
		struct VideoSettings {
			short imageWidth;
			short imageHeight;
			int imgType;
			cv::Rect crop;
		};
	private:
		VideoSettings settings;
		cv::VideoCapture* video;
		
	public:
		cv::Mat frame; // сам кадр видео в родном формате
		IInputProvider(std::string filename, VideoSettings settings);
		bool GetNextFrame(cv::Mat& dst); //получить следующий кадр из видео
		void SetVideoPos(double ms);
		double GetVideoPos(void);
	};

	//=======================================================================================================
	//
	//                         Базовые классы архитектуры телеавтомата
	//
	//=======================================================================================================

	//детектор уникальных элементов на изображении
	//работает по данному кадру. не имеет памяти
	class Primary {
	public : 
		virtual fcs::TPrimary Process(const cv::Mat& input) { fcs::TPrimary t; return t; };
	};

	//Вторичка которая осуществляет сопоставление уникальных элементов на соседних кадрах
	//имеет память максимум на предыдущий кадр
	class Secondary {
	public:
		virtual std::vector<fcs::object> Process(fcs::TPrimary primary, fcs::Control& ctrl) { std::vector<fcs::object> o; return o; };
	};

	//Трэкер Осуществляет траеторный анализ движения объектов на нескольких кадрах
	//память  на 3 и более кадра
	class Tracker {
	public:
		virtual std::vector<fcs::object> Proccess(std::vector<fcs::object>) {};
	};
};