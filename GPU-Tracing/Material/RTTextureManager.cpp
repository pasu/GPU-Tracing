#include "RTTextureManager.h"


RTTextureManager::RTTextureManager()
{

}

RTTextureManager::~RTTextureManager()
{
	ClearAll();
}

int RTTextureManager::CreateTexture( string strFileName)
{
	RTTexture *pTexture = NULL;

    vector<string>::iterator it = find( mKeys.begin(), mKeys.end(), strFileName );

	int idx = -1;

	if ( it != mKeys.end() )
	{
		idx = std::distance( mKeys.begin(), it );
		return idx;
	}

    pTexture = new RTTexture();
	pTexture->LoadTextureImage( strFileName.c_str() );

    mTextures.push_back( pTexture );
	mKeys.push_back( strFileName );

	return mTextures.size()-1;
}

int RTTextureManager::generateTexInfoWithBuffer( vector<RTTexInfo> &texInfos, unsigned char *&fBuffer )
{
	int offset = 0;
	int idx = 0;
	for ( RTTexture *pTexture : mTextures )
	{
		RTTexInfo info;
		info.idx = idx++;

		info.offset = offset;
		offset += pTexture->getBufferLength();

        info.width = pTexture->m_Width;
		info.height = pTexture->m_Height;

        texInfos.push_back( info );
	}

	fBuffer = new unsigned char[offset];
    offset = 0;
    for ( RTTexture *pTexture : mTextures )
	{
		int length = pTexture->getBufferLength();
		memcpy( fBuffer + offset, pTexture->m_Buffer, length );
		offset += length;
	}

    return offset;
}

void RTTextureManager::ClearAll()
{
	for ( RTTexture* pTexture : mTextures )
	{
		delete pTexture;
		pTexture = NULL;
	}

	mTextures.clear();
}
