#include "RTTexture.h"
#include "FreeImage/FreeImage.h"

RTTexture::RTTexture()
{
	m_Buffer = NULL;
}

RTTexture::~RTTexture()
{
	if ( m_Buffer )
	{
		delete m_Buffer;
		m_Buffer = NULL;
	}
}

void RTTexture::LoadTextureImage( const char *a_File )
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType( a_File, 0 );
	if ( fif == FIF_UNKNOWN ) fif = FreeImage_GetFIFFromFilename( a_File );
	FIBITMAP *tmp = FreeImage_Load( fif, a_File );
	FIBITMAP *dib = FreeImage_ConvertTo32Bits( tmp );
	FreeImage_Unload( tmp );
	m_Width = FreeImage_GetWidth( dib );
	m_Height = FreeImage_GetHeight( dib );
	m_Buffer = new vec3[m_Width * m_Height];

	for ( int y = 0; y < m_Height; y++ )
	{
		unsigned char *line = FreeImage_GetScanLine( dib, m_Height - 1 - y );
		for ( int x = 0; x < m_Width; x++ )
		{
			int basePixel = x * 4;
			unsigned char *data = (unsigned char *)line;
			m_Buffer[y * m_Width + x] = {data[basePixel + 2] / 255.0f, data[basePixel + 1] / 255.0f, data[basePixel] / 255.0f};
		}
	}
	FreeImage_Unload( dib );
}

int RTTexture::getBufferLength() const
{
	return m_Width * m_Height * sizeof( vec3 );
}
