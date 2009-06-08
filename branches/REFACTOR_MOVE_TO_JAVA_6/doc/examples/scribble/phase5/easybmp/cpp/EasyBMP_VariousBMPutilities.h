/*************************************************
*                                                *
*  EasyBMP Cross-Platform Windows Bitmap Library * 
*                                                *
*  Author: Paul Macklin                          *
*   email: macklin01@users.sourceforge.net       *
* support: http://easybmp.sourceforge.net        *
*                                                *
*          file: EasyBMP_VariousBMPutilities.h   *
*    date added: 05-02-2005                      *
* date modified: 02-06-2006                      *
*       version: 1.00                            *
*                                                *
*   License: BSD (revised/modified)              *
* Copyright: 2005-6 by the EasyBMP Project       * 
*                                                *
* description: Various utilities.                *
*                                                *
*************************************************/

#ifndef _EasyBMP_VariousBMPutilities_h_
#define _EasyBMP_VariousBMPutilities_h_

BMFH GetBMFH( const char* szFileNameIn );
BMIH GetBMIH( const char* szFileNameIn );
void DisplayBitmapInfo( const char* szFileNameIn );
int GetBitmapColorDepth( const char* szFileNameIn );
void PixelToPixelCopy( BMP& From, int FromX, int FromY,  
                       BMP& To, int ToX, int ToY);
void PixelToPixelCopyTransparent( BMP& From, int FromX, int FromY,  
                                  BMP& To, int ToX, int ToY,
                                  RGBApixel& Transparent );
void RangedPixelToPixelCopy( BMP& From, int FromL , int FromR, int FromB, int FromT, 
                             BMP& To, int ToX, int ToY );
void RangedPixelToPixelCopyTransparent( 
     BMP& From, int FromL , int FromR, int FromB, int FromT, 
     BMP& To, int ToX, int ToY ,
     RGBApixel& Transparent );
bool CreateGrayscaleColorTable( BMP& InputImage );

BMFH GetBMFH( const char* szFileNameIn )
{
 using namespace std;
 BMFH bmfh;

 FILE* fp;
 fp = fopen( szFileNameIn,"rb");
 
 if( !fp  )
 {
  cout << "EasyBMP Error: Cannot initialize from file " 
       << szFileNameIn << "." << endl
       << "               File cannot be opened or does not exist." 
	   << endl;
  bmfh.bfType = 0;
  return bmfh;
 } 
 
 SafeFread( (char*) &(bmfh.bfType) , sizeof(WORD) , 1 , fp );
 SafeFread( (char*) &(bmfh.bfSize) , sizeof(DWORD) , 1 , fp ); 
 SafeFread( (char*) &(bmfh.bfReserved1) , sizeof(WORD) , 1 , fp ); 
 SafeFread( (char*) &(bmfh.bfReserved2) , sizeof(WORD) , 1 , fp ); 
 SafeFread( (char*) &(bmfh.bfOffBits) , sizeof(DWORD) , 1 , fp ); 
 
 fclose( fp );
 
 if( IsBigEndian() )
 {
  bmfh.SwitchEndianess();
 }

 return bmfh;
}

BMIH GetBMIH( const char* szFileNameIn )
{
 using namespace std;
 BMFH bmfh;
 BMIH bmih;

 FILE* fp;
 fp = fopen( szFileNameIn,"rb");

 if( !fp  )
 {
  cout << "EasyBMP Error: Cannot initialize from file " 
       << szFileNameIn << "." << endl
       << "               File cannot be opened or does not exist." 
	   << endl;
  return bmih;
 } 
 
 // read the bmfh, i.e., first 14 bytes (just to get it out of the way);
 
 BYTE TempBYTE;
 int i;
 for( i = 14 ; i > 0 ; i-- )
 {
  SafeFread( (char*) &TempBYTE , sizeof(BYTE) , 1, fp );
 }

 // read the bmih 

 SafeFread( (char*) &(bmih.biSize) , sizeof(DWORD) , 1 , fp );
 SafeFread( (char*) &(bmih.biWidth) , sizeof(DWORD) , 1 , fp ); 
 SafeFread( (char*) &(bmih.biHeight) , sizeof(DWORD) , 1 , fp ); 
 SafeFread( (char*) &(bmih.biPlanes) , sizeof(WORD) , 1 , fp ); 
 
 SafeFread( (char*) &(bmih.biBitCount) , sizeof(WORD) , 1 , fp );
 SafeFread( (char*) &(bmih.biCompression) , sizeof(DWORD) , 1 , fp ); 
 SafeFread( (char*) &(bmih.biSizeImage) , sizeof(DWORD) , 1 , fp ); 
 SafeFread( (char*) &(bmih.biXPelsPerMeter) , sizeof(DWORD) , 1 , fp ); 
 
 SafeFread( (char*) &(bmih.biYPelsPerMeter) , sizeof(DWORD) , 1 , fp ); 
 SafeFread( (char*) &(bmih.biClrUsed) , sizeof(DWORD) , 1 , fp ); 
 SafeFread( (char*) &(bmih.biClrImportant) , sizeof(DWORD) , 1 , fp ); 
 
 fclose( fp );
 
 if( IsBigEndian() )
 {
  bmih.SwitchEndianess();
 }

 return bmih;
}

void DisplayBitmapInfo( const char* szFileNameIn )
{
 using namespace std;
 FILE* fp;
 fp = fopen( szFileNameIn,"rb");
 
 if( !fp  )
 {
  cout << "EasyBMP Error: Cannot initialize from file " 
       << szFileNameIn << "." << endl
       << "               File cannot be opened or does not exist." 
	   << endl;
  return;
 } 
 fclose( fp );

 // don't duplicate work! Just use the functions from above!
 
 BMFH bmfh = GetBMFH(szFileNameIn);
 BMIH bmih = GetBMIH(szFileNameIn);

 cout << "File information for file " << szFileNameIn 
      << ":" << endl << endl;

 cout << "BITMAPFILEHEADER:" << endl
      << "bfType: " << bmfh.bfType << endl
      << "bfSize: " << bmfh.bfSize << endl
      << "bfReserved1: " << bmfh.bfReserved1 << endl
      << "bfReserved2: " << bmfh.bfReserved2 << endl    
      << "bfOffBits: " << bmfh.bfOffBits << endl << endl;

 cout << "BITMAPINFOHEADER:" << endl
      << "biSize: " << bmih.biSize << endl
      << "biWidth: " << bmih.biWidth << endl
      << "biHeight: " << bmih.biHeight << endl
      << "biPlanes: " << bmih.biPlanes << endl
      << "biBitCount: " << bmih.biBitCount << endl
      << "biCompression: " << bmih.biCompression << endl
      << "biSizeImage: " << bmih.biSizeImage << endl
      << "biXPelsPerMeter: " << bmih.biXPelsPerMeter << endl
      << "biYPelsPerMeter: " << bmih.biYPelsPerMeter << endl
      << "biClrUsed: " << bmih.biClrUsed << endl
      << "biClrImportant: " << bmih.biClrImportant << endl << endl;  
 return;
}

int GetBitmapColorDepth( const char* szFileNameIn )
{
 BMIH bmih = GetBMIH( szFileNameIn );
 
 return (int) bmih.biBitCount;
}

void PixelToPixelCopy( BMP& From, int FromX, int FromY,  
                       BMP& To, int ToX, int ToY)
{
 *To(ToX,ToY) = *From(FromX,FromY);
 return;
}

void PixelToPixelCopyTransparent( BMP& From, int FromX, int FromY,  
                                  BMP& To, int ToX, int ToY,
                                  RGBApixel& Transparent )
{
 if( From(FromX,FromY)->Red != Transparent.Red ||
     From(FromX,FromY)->Green != Transparent.Green ||
     From(FromX,FromY)->Blue != Transparent.Blue )	 
 {
  *To(ToX,ToY) = *From(FromX,FromY);
 }
 return;
}

void RangedPixelToPixelCopy( BMP& From, int FromL , int FromR, int FromB, int FromT, 
                             BMP& To, int ToX, int ToY )
{
 // make sure the conventions are followed
 if( FromB < FromT )
 { int Temp = FromT; FromT = FromB; FromB = Temp; }

 // make sure that the copied regions exist in both bitmaps
 if( FromR >= From.TellWidth() )
 { FromR = From.TellWidth()-1; }
 if( FromL < 0 ){ FromL = 0; }

 if( FromB >= From.TellHeight() )
 { FromB = From.TellHeight()-1; }
 if( FromT < 0 ){ FromT = 0; }
 
 if( ToX+(FromR-FromL) >= To.TellWidth() )
 { FromR = To.TellWidth()-1+FromL-ToX; }
 if( ToY+(FromB-FromT) >= To.TellHeight() )
 { FromB = To.TellHeight()-1+FromT-ToY; } 
 
 int i,j;
 for( j=FromT ; j <= FromB ; j++ )
 { 
  for( i=FromL ; i <= FromR ; i++ )
  {
   PixelToPixelCopy( From, i,j,  
                     To, ToX+(i-FromL), ToY+(j-FromT) );
  }
 }

 return;
}

void RangedPixelToPixelCopyTransparent( 
     BMP& From, int FromL , int FromR, int FromB, int FromT, 
     BMP& To, int ToX, int ToY ,
     RGBApixel& Transparent )
{
 // make sure the conventions are followed
 if( FromB < FromT )
 { int Temp = FromT; FromT = FromB; FromB = Temp; }

 // make sure that the copied regions exist in both bitmaps
 if( FromR >= From.TellWidth() )
 { FromR = From.TellWidth()-1; }
 if( FromL < 0 ){ FromL = 0; }

 if( FromB >= From.TellHeight() )
 { FromB = From.TellHeight()-1; }
 if( FromT < 0 ){ FromT = 0; }
 
 if( ToX+(FromR-FromL) >= To.TellWidth() )
 { FromR = To.TellWidth()-1+FromL-ToX; }
 if( ToY+(FromB-FromT) >= To.TellHeight() )
 { FromB = To.TellHeight()-1+FromT-ToY; } 
 
 int i,j;
 for( j=FromT ; j <= FromB ; j++ )
 { 
  for( i=FromL ; i <= FromR ; i++ )
  {
   PixelToPixelCopyTransparent( From, i,j,  
                     To, ToX+(i-FromL), ToY+(j-FromT) , 
                     Transparent);
  }
 }

 return;
}

bool CreateGrayscaleColorTable( BMP& InputImage )
{
 using namespace std;
 int BitDepth = InputImage.TellBitDepth();
 if( BitDepth != 1 && BitDepth != 4 && BitDepth != 8 )
 {
  cout << "EasyBMP Warning: Attempted to create color table at a bit" << endl
       << "                 depth that does not require a color table." << endl
 	   << "                 Ignoring request." << endl;  
  return false;
 }
 int i;
 int NumberOfColors = InputImage.TellNumberOfColors();
 
 BYTE StepSize;
 if( BitDepth != 1 )
 { StepSize = 255/(NumberOfColors-1); }
 else
 { StepSize = 255; }
  
 for( i=0 ; i < NumberOfColors ; i++ )
 {
  BYTE TempBYTE = i*StepSize;
  RGBApixel TempColor;
  TempColor.Red = TempBYTE;
  TempColor.Green = TempBYTE;
  TempColor.Blue = TempBYTE;
  TempColor.Alpha = 0;
  InputImage.SetColor( i , TempColor );  
 }
 return true;
}

#endif
