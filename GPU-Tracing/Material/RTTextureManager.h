#pragma once
#include "RTTexture.h"

#include <string>
#include <vector>

using namespace std;

struct RTTexInfo
{
	int idx;
	int offset;
	int width;
	int height;
};

class RTTextureManager
{
  public:
	RTTextureManager();
	~RTTextureManager();

	int CreateTexture( string strFileName) ;

    int generateTexInfoWithBuffer( vector<RTTexInfo> &texInfos, unsigned char *&fBuffer );

	void ClearAll();
  private:
	vector<string> mKeys;
	vector<RTTexture *> mTextures;

};
