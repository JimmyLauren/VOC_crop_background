
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include "io.h"
#include "opencv2/core/core.hpp" 
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <algorithm>

//tinyXML
#include <tinyxml.h>


#include <map>

#include <windows.h>
using namespace std;
using namespace cv;

//�ж����������Ƿ��غ�
/*
-------------------- -
���ߣ�Bevision
��Դ��CSDN
ԭ�ģ�https ://blog.csdn.net/bevison/article/details/33730909
��Ȩ����������Ϊ����ԭ�����£�ת���븽�ϲ������ӣ�
*/
float DecideOverlap(const Rect r1, const Rect r2)
{
	int x1 = r1.x;
	int y1 = r1.y;
	int width1 = r1.width;
	int height1 = r1.height;

	int x2 = r2.x;
	int y2 = r2.y;
	int width2 = r2.width;
	int height2 = r2.height;

	int endx = max(x1 + width1, x2 + width2);
	int startx = min(x1, x2);
	int width = width1 + width2 - (endx - startx);

	int endy = max(y1 + height1, y2 + height2);
	int starty = min(y1, y2);
	int height = height1 + height2 - (endy - starty);

	float ratio = 0.0f;
	float Area, Area1, Area2;

	if (width <= 0 || height <= 0)
		return 0.0f;
	else
	{
		Area = width*height;
		Area1 = width1*height1;
		Area2 = width2*height2;
		ratio = Area / (Area1 + Area2 - Area);
	}
	return ratio;
}


//����tinyXML����xml�ļ�����xml�ļ��е����С�object���е�λ������ѹջ
enum SuccessEnum { FAILURE, SUCCESS };
SuccessEnum loadXML(const string & xFile,vector<cv::Rect> & locations)
{
	TiXmlDocument doc;
	if (!doc.LoadFile(xFile.c_str()))
	{
		cerr << doc.ErrorDesc() << endl;
		return FAILURE;
	}

	TiXmlElement* root = doc.FirstChildElement();
	if (root == NULL)
	{
		cerr << "Failed to load file: No root element." << endl;
		doc.Clear();
		return FAILURE;
	}

	for (TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		cv::Rect tempRect;
		string elemName = elem->Value();
		bool k1 = strcmp(elemName.c_str(), "object");
		if (!k1) {
			for (TiXmlElement* elem_c1 = elem->FirstChildElement(); elem_c1 != NULL; elem_c1 = elem_c1->NextSiblingElement()) {
				string objectC = elem_c1->Value();
				bool k2=strcmp(objectC.c_str(), "bndbox");
				if (!k2) {
					string xmin, ymin, xmax, ymax;
					for (TiXmlElement* elem_c2 = elem_c1->FirstChildElement(); elem_c2 != NULL; elem_c2 = elem_c2->NextSiblingElement()) {
						string bnd = elem_c2->Value();
						if (!strcmp(bnd.c_str(), "xmin"))
							xmin = elem_c2->FirstChild()->ToText()->Value();
						if (!strcmp(bnd.c_str(), "ymin"))
							ymin = elem_c2->FirstChild()->ToText()->Value();
						if (!strcmp(bnd.c_str(), "xmax"))
							xmax = elem_c2->FirstChild()->ToText()->Value();
						if (!strcmp(bnd.c_str(), "ymax"))
							ymax = elem_c2->FirstChild()->ToText()->Value();
					}
					tempRect.x=atoi(xmin.c_str());
					tempRect.y = atoi(ymin.c_str());
					tempRect.width = atoi(xmax.c_str())- atoi(xmin.c_str());
					tempRect.height = atoi(ymax.c_str())- atoi(ymin.c_str());
					locations.push_back(tempRect); //ѹջ
					//string xmin = elem_c1->Attribute("xmin");
					//string ymin = elem_c1->Attribute("ymin");
					//string xmax = elem_c1->Attribute("xmax");
					//string ymax = elem_c1->Attribute("ymax");
				}
			}
		}


		//const char* attr;
		//attr = elem->Attribute("priority");
		//if (strcmp(attr, "1") == 0)
		//{
		//	TiXmlElement* e1 = elem->FirstChildElement("bold");
		//	TiXmlNode* e2 = e1->FirstChild();
		//	cout << "priority=1\t" << e2->ToText()->Value() << endl;

		//}
		//else if (strcmp(attr, "2") == 0)
		//{
		//	TiXmlNode* e1 = elem->FirstChild();
		//	cout << "priority=2\t" << e1->ToText()->Value() << endl;
		//}
	}
	doc.Clear();
	return SUCCESS;
}



void getAllFiles(const string path, const string ext, vector<string>& files)
/*
---------------------
���ߣ�SnailTyan
��Դ��CSDN
ԭ�ģ�https ://blog.csdn.net/Quincuntial/article/details/50058415
��Ȩ����������Ϊ����ԭ�����£�ת���븽�ϲ������ӣ�
*/
{
	cout << path << endl;
	cout << ext << endl;
	//�ļ����
	intptr_t hFile = 0;
	struct _finddata_t fileInfo;
	string p;

	if ((hFile = _findfirst(p.assign(path).append("\\*").append(ext).c_str(), &fileInfo)) != -1)
	{
		do
		{
			files.push_back(fileInfo.name);
		} while (_findnext(hFile, &fileInfo) == 0);
		_findclose(hFile);
	}
}


void getFiles(std::string path, std::vector<std::string>& files)
{
	//�ļ����  
	intptr_t   hFile = 0;
	//�ļ���Ϣ  
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("/*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))  //�����Ŀ¼,����֮
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("/").append(fileinfo.name), files);
			}
			else  //�������,�����б�  
			{
				string fname(fileinfo.name);
				string imgType = fname.substr(fname.rfind("."), fname.length());
				if (imgType == ".jpg"|| imgType == ".jpeg")
				{
					files.push_back(p.assign(path).append("/").append(fileinfo.name));  //ԭ����ֻ����伴��
				}
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

void getJustCurrentDir(string path, vector<string>& files)
{
	//�ļ���� 
	long  hFile = 0;
	//�ļ���Ϣ 
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					files.push_back(fileinfo.name);
					//files.push_back(p.assign(path).append("\\").append(fileinfo.name) );
				}

			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}



int main()
{
	//IplImage*    pLImage = NULL;
	Mat Image;
	//---------------------------------------  ����ͼƬ-------------------------//
	string filePath = "F:/DATASETS/Car_Detection/ShiPinJieGouHua-RenCheJianChe/FinalData/20181019/JPEGimages";
	string xmlFileStr = "F:/DATASETS/Car_Detection/ShiPinJieGouHua-RenCheJianChe/FinalData/20181019/Annotations";
	string savePath = "F:\\DATASETS\\background";
	vector<string> files;
	getAllFiles(filePath, ".jpg", files);
	//getFiles(filePath, files);

	cout << files.size() << endl;

	vector<cv::Rect> rects;
	int imgProc = 0;
	for (int i = 0; i < files.size(); i++)
	{
		int count = 0;
		imgProc++;

		rects.clear();
		//string temp = files[i].substr(files[i].rfind("/") + 1, files[i].length() - files[i].rfind("/"));
		//cout << temp << endl;
		string pureName = files[i].substr(0, files[i].length()-4);
		//cout << pureName << endl;
		
		//pLImage = cvLoadImage(files[i].c_str(), CV_LOAD_IMAGE_COLOR);
		string img_path = filePath + "/" + files[i];
		//cout << img_path << endl;
		Image = imread(img_path.c_str(), CV_LOAD_IMAGE_COLOR);
		//if (!pLImage)
		//{
		//	printf("open pLImage error!\n");
		//	return 1;
		//}

		//��ȡxml�ļ�����ȡ����Ŀ���λ��
		string xmlFile = xmlFileStr + "/" + pureName + ".xml";

		//�ж��Ƿ����xml�ļ��������������
		if (-1==(_access(xmlFile.c_str(), 0))) {
			cout<<xmlFile<<" does not exists!"<<endl;
			continue;
		}

		loadXML(xmlFile, rects); //�õ����еı�עλ��

		//cout << temp3 << endl;
		for (int i=0; i<10; i++)
		{
			//for (int n = 0; n < 20; n=n+10)
			//{
			Mat ROIImage;
			Rect rect;
			rect.x = rand()%Image.cols;
			rect.y = rand()%Image.rows;
			rect.width = 64 + rand() % 136;
			rect.height = 64 + rand() % 136;

			//Խ�紦��
			if (rect.x + rect.width > Image.cols)
				rect.width = Image.cols - rect.x;
			if (rect.y + rect.height > Image.rows)
				rect.height = Image.rows - rect.y;
			if (rect.height < 40 || rect.width < 40)
				continue;

			//�ж��Ƿ��������Ƿ��غ�
			bool isOverlap = false;
			for (vector<cv::Rect>::iterator it = rects.begin(); it != rects.end(); it++) {
				float ratio = DecideOverlap(*it, rect);
				//cout << ratio << endl;
				if (ratio > 0.001) {//����������غϣ����غϱ�־Ϊtrue,��ʾrect�и�rects�еĿ��غ�
					isOverlap = true;
					break;
				}
			}
			if (!isOverlap) {
				Image(rect).copyTo(ROIImage);

				imshow("image", ROIImage);
				waitKey(50);


				//string temp = filename[index];
				//string name2 = filePath + stringName[index] + "\\" + filename[index];

				string save2 = savePath+"/"+to_string(i) + "_" + pureName+".jpg";
				cv::imwrite(save2, ROIImage);
				count++;
				//cvSaveImage(save2.c_str(), ROIImage);
				//}
			}
			//cvSetImageROI(pLImage, rect);
			//ROI_char = cvCreateImage(cvSize(rect.width, rect.height), pLImage->depth, pLImage->nChannels);
			//cvCopy(pLImage, ROI_char);

		}
		cout << count << " images have been cropped!!!" << endl;
		cout << "----------------" << imgProc << " totally!!" << endl;
	}
	//system("pause");
	return 0;
}
