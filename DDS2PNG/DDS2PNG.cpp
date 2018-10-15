// DDS2PNG.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <io.h>
#include <direct.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include<Windows.h>

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <osgDB\ReadFile>
#include <osgDB\WriteFile>
#include <osg\Node>
#include <osg\Matrix>
#include <osg\MatrixTransform>
#include <osgUtil\Optimizer>

using namespace cv;
using namespace osg;
using namespace std;

//根据字符串执行CMD程序
void ExcuteCmd(LPSTR cs)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		PROCESS_INFORMATION pInfo;
		STARTUPINFO sInfo = { sizeof(STARTUPINFO) };
		sInfo.hStdError = hWrite;
		sInfo.hStdOutput = hWrite;
		sInfo.wShowWindow = SW_HIDE;
		sInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		CreateProcess(NULL, cs, NULL,NULL,TRUE, NULL, NULL, NULL, &sInfo, &pInfo);
		WaitForSingleObject(pInfo.hProcess, INFINITE);
	}
	CloseHandle(hWrite);
}
//读取所有的osg文件
void GetFilePath(string filePath, vector<string> &pathVector, string flag)
{
	const char* dir = filePath.c_str();
	_chdir(dir);
	long hFile;
	_finddata_t fileinfo;
	if ((hFile = _findfirst("*.*", &fileinfo)) != -1)
	{
		do
		{
			if (!(fileinfo.attrib&_A_SUBDIR))
			{
				string path = string(fileinfo.name);
				string suffic = path.substr(path.find_first_of(".") + 1);
				if (strcmp(suffic.c_str(), flag.c_str()) == 0)
				{
					char filename[_MAX_PATH];
					string tempname = filePath + "\\" + path;
					pathVector.emplace_back(tempname);
				}
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}
//创建临时文件
void CopyFile(string srcFile, string destFile)
{
	ifstream infile;
	ofstream outfile;
	string temp;
	infile.open(srcFile, ios::in);
	outfile.open(destFile, ios::out | ios::trunc);
	if (infile.good())
	{
		while (!infile.eof())
		{
			getline(infile, temp, '\n');
			outfile << temp << '\n';
		}
	}
	infile.close();
	outfile.close();
}
//读取mtl文件重新设置纹理后缀
void ResetSuffix(string file)
{
	string savePath = file.substr(0, file.find_last_of("\\"));
	string bup = savePath + "\\tem.mtl";
	CopyFile(file, bup);
	ifstream infile;
	ofstream outfile;
	infile.open(bup, ios::in);
	outfile.open(file, ios::out | ios::trunc);

	string str;
	while (!infile.eof())
	{
		getline(infile, str, '\n');
		size_t pos = str.find("dds");
		if (pos != -1)
		{
			str.replace(pos, pos + 3, "png");
			outfile << str << '\n';
		}
		else
			outfile << str << '\n';
	}
	infile.close();
	outfile.close();
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		printf("参数个数少于3个");
		getchar();
		return 0;
	}
	string fileExePath = argv[0];//程序执行全路径
	string s1 = "执行程序路径为:" + fileExePath + '\n';
	printf(s1.c_str());
	fileExePath = fileExePath.substr(0, fileExePath.find_last_of("\\"));
	const string filePath = argv[1];
	string s2 = "文件路径为:" + filePath + '\n';
	printf(s2.c_str());
	const string outPath = argv[2];
	string s3 = "输出文件路径为:" + outPath + "\n";
	printf(s3.c_str());
	const string osgconv = fileExePath + "\\osgconv.exe";
	const string texconv = fileExePath + "texconv.exe";
	const string strSuffixObj = ".obj";
	const string strSuffixMtl = ".mtl";
	const string strOsg = "osg";
	const string strDds = "dds";
	const string strPng = "PNG";
	//1.将所有的输入路径下的.osg文件转换为obj输出
	vector<string> pathVector;
	GetFilePath(filePath, pathVector, strOsg);
	vector<string>::iterator iterBegin = pathVector.begin();
	vector<string>::iterator iterEnd = pathVector.end();
	size_t len = pathVector.size();
	printf("osg文件个数为%d\n", len);
	for (vector<string>::iterator iter = iterBegin; iter != iterEnd; ++iter)
	{
		string osgPath = *iter;
		string tempName = osgPath.substr(osgPath.find_last_of('\\') + 1);
		tempName = tempName.substr(0, tempName.find_last_of('.'));
		string outName = outPath + "\\" + tempName + strSuffixObj;

		osg::ref_ptr<Node> node = osgDB::readNodeFile(osgPath);
		if (node == NULL)
			continue;
		BoundingSphere bs = node->getBound();
		Matrix m;
		m.makeIdentity();
		m = Matrix::translate(bs.center());

		osg::ref_ptr<MatrixTransform> trans = new MatrixTransform;
		trans->setDataVariance(osg::Object::STATIC);
		//绕X轴旋转90
		trans->setMatrix(m*Matrix::rotate(osg::DegreesToRadians(-90.0f), osg::Vec3(1, 0, 0)));
		trans->addChild(node);
		osgDB::Registry::instance()->writeNode(*trans, outName, osgDB::Registry::instance()->getOptions());
		//此处也可以调用osgconv.exe执行cmd操作

		//2.修改所有的mtl文件中的dds后缀为png
		string mtlPath = outPath + "\\" + tempName + strSuffixMtl;
		ResetSuffix(mtlPath);
	}
	//3.将输出路径下的所有dds文件转换为png文件
	pathVector.clear();
	GetFilePath(filePath, pathVector, strDds);
	iterBegin = pathVector.begin();
	iterEnd = pathVector.end();
	len = pathVector.size();
	printf("dds文件个数为%d\n", len);
	for (vector<string>::iterator iter = iterBegin; iter != iterEnd; ++iter)
	{
		string ddspath = *iter;
		string ddsname = ddspath.substr(ddspath.find_last_of('\\') + 1);
		ddsname = ddsname.substr(0, ddsname.find_last_of('.'));
		string cmd = texconv + " " + ddspath + " -ft png -o" + outPath + " -y";
		LPSTR wCmd = (LPSTR)cmd.c_str();
		ExcuteCmd(wCmd);
	}
	//4.将所有的图片进行镜像处理
	pathVector.clear();
	GetFilePath(outPath, pathVector, strPng);
	iterBegin = pathVector.begin();
	iterEnd = pathVector.end();
	len = pathVector.size();
	printf("png文件个数为%d\n", len);
	for (vector<string>::iterator iter = iterBegin; iter != iterEnd; ++iter)
	{
		Mat src = cv::imread(*iter, CV_LOAD_IMAGE_UNCHANGED);
		Mat dist;
		dist.create(src.rows, src.cols, src.type());
		int rows = src.rows;
		for (int i = 0; i < rows; ++i)
		{
			src.row(rows - i - 1).copyTo(dist.row(i));
		}
		imwrite(*iter, dist);
	}
	//5.删除临时文件
	string deleteTempFile = outPath + "\\tmp.mtl";
	DeleteFile(deleteTempFile.c_str());

	printf("GAME OVER!\n");
	getchar();
	return 1;
}

