#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <ctime>
#include <windows.h>

using namespace std;


/*
**1.ʹ��MMX�����Ĵ���
**/

 int Mmx(LPBYTE Picture1,LPBYTE Picture2,LPBYTE Picture,
	 int intWidth,int intHeight,int RGB_Bit,int i){
	 int x,y;
	 LPDWORD temp1,temp2,temp;
	 int fade_Rate = i*128;					//��fadeֵ��չΪ16λ������ӦMMX��16λ����
	 WORD fade1[4], fade2[4];
	 fade1[0] =fade1[1] =fade1[2] =fade1[3] =32767 -fade_Rate ;//16λ�����������Ϊ32767
	 fade2[0] =fade2[1] =fade2[2] =fade2[3] = fade_Rate;
	 for(y=0;y<intHeight;y++){
		  temp1=(LPDWORD)(Picture1+intWidth*RGB_Bit/8*y);
		  temp2=(LPDWORD)(Picture2+intWidth*RGB_Bit/8*y);
		  temp =(LPDWORD)(Picture+intWidth*RGB_Bit/8*y);
		  for(x=0;x<intWidth;x++){
			   _asm{		//Result_pixel=A_pixel*fade+B_pixel*(1-fade)=(A-B)*fade+B
					 pxor  mm7,mm7			//��mm7�Ĵ������Ϊ0
					 movq  mm2,[fade1]		//����ֵװ��mm2�Ĵ���
					 movq  mm3,[fade2]
					 mov   esi,[temp1]
					 mov   edx,[temp2]
					 mov   edi,[temp]
					 movd  mm0,[esi]		//ȡͼ��1�����ط���װ��mm0�Ĵ���
					 movd  mm1,[edx]
					 punpcklbw mm0,mm7
					 punpcklbw mm1,mm7		//���ֽڽ������16λ
					 pmulhw  mm0,mm2		//���Խ�ֵ  ��a*fade��
					 pmulhw  mm1,mm3		//(b*(1-fade))
					 paddw  mm0,mm1
					 packuswb mm0,mm7		//����16λ���ֽ�
					 movd  [edi],mm0}
			   temp1++;
			   temp2++;
			   temp++;}
	}_asm EMMS
	return 0;}

 /*
**2.δʹ��MMX �����Ĵ���
**/
         

int NoMmx(LPBYTE Picture1,LPBYTE Picture2,LPBYTE Picture,
		  int intWidth,int intHeight,int RGB_Bit,int i){
	 int x,y;
	 LPBYTE temp1,temp2,temp;
	 BYTE fade_Rate = (BYTE)i;          //��fadeֵ��չΪ16λ������ӦMMX��16λ����
	 BYTE fade1, fade2;
	 fade1 =255 -fade_Rate ;			//16λ�����������Ϊ32767
	 fade2 = fade_Rate;
	 for(y=0;y<intHeight;y++){
		  temp1=(LPBYTE)(Picture1+intWidth*RGB_Bit/8*y);
		  temp2=(LPBYTE)(Picture2+intWidth*RGB_Bit/8*y);
		  temp =(LPBYTE)(Picture+intWidth*RGB_Bit/8*y);
		  for(x=0;x<3*intWidth;x++){
			   _asm{
				   mov dh,[fade1]
				   mov dl,[fade2]
				   mov ecx,[temp1]
				   mov bh,[ecx]
				   mov esi,[temp2]
				   mov bl,[esi]
				   mov al,dh
				   mul bh
				   mov edi,[temp]
				   mov [edi],ah
				   mov al,dl
				   mul bl
				   add ah,[edi]
				   mov [edi],ah}
			   temp1++;
			   temp2++;
			   temp++;}
	}_asm EMMS
	return 0;}

void Test_MMX(int intWidth, int intHeight, int RGB_Bit, int Time)
{
    _try{_asm EMMS}
    _except(EXCEPTION_EXECUTE_HANDLER){}
    HDC hDC;
    LPBITMAPINFO lpInfo;						//ָ��ͼƬ����ĳ�ָ������
    int intSize,i;
    LPBYTE  lpBuf1,lpBuf2,lpBuf;				//ָ���ֽ�(��)��ָ��
    HFILE Picture1,Picture2;
    Picture1=_lopen("C:\\Users\\zc12345\\Pictures\\mmx1.bmp",OF_READ);		//���ش��ļ��ľ��
    Picture2=_lopen("C:\\Users\\zc12345\\Pictures\\mmx2.bmp",OF_READ);
    intSize=GetFileSize((HANDLE)Picture1,NULL);	//�����ļ�����

    lpBuf1=(LPBYTE)LocalAlloc(LPTR,intSize);	//�����·�����ڴ��ַ,,�Ӷ��з���ָ����С���ֽ���
    lpBuf2=(LPBYTE)LocalAlloc(LPTR,intSize);
    lpBuf=(LPBYTE)LocalAlloc(LPTR,intSize);

    _lread(Picture1,lpBuf1,intSize);			//ָ��һ���ڴ���ָ�몢���ݽ���������ڴ��
    _lread(Picture2,lpBuf2,intSize);
    _lclose(Picture1);
    _lclose(Picture2);
    memcpy(lpBuf,lpBuf1,intSize);
    lpInfo = (LPBITMAPINFO)(lpBuf+0x0e);
    hDC = GetDC(NULL);
	DWORD start_time1=GetTickCount();
    for(i=0;i<255;i++)							//Ĭ����ͼ��255�׽���,����fade��ֵ�仯255��
    {
		Mmx(lpBuf1+0x60,lpBuf2+0x60,lpBuf+0x60,intWidth,intHeight,RGB_Bit,i);
		//Sleep(Time);							//ָ����lpBuf+0x60��һ�п�ʼɨ��
		SetDIBitsToDevice(hDC,0,0,intWidth,intHeight,0,0,0,480,lpBuf+0x60,lpInfo,DIB_RGB_COLORS);
												//�ú���ʹ��DIBλͼ����ɫ���ݶ���Ŀ���豸������ص��豸
												//�ϵ�ָ�������е����ؽ������á�
    }
	DWORD end_time1=GetTickCount();
	cout<<"ʹ��MMX��ʱ:"<<(end_time1-start_time1)<<"ms!"<<endl;//�������ʱ��

	DWORD start_time2=GetTickCount();
    for(i=0;i<255;i++)						
    {
		NoMmx(lpBuf1+0x60,lpBuf2+0x60,lpBuf+0x60,intWidth,intHeight,RGB_Bit,i);
		SetDIBitsToDevice(hDC,640,0,intWidth,intHeight,0,0,0,480,lpBuf+0x60,lpInfo,DIB_RGB_COLORS);
    }
	DWORD end_time2=GetTickCount();
	cout<<"��ʹ��MMX��ʱ:"<<(end_time2-start_time2)<<"ms!"<<endl;
    LocalFree(lpBuf1);
    LocalFree(lpBuf2);
    ReleaseDC(NULL,hDC);
}


int _tmain(int argc, _TCHAR* argv[])
{
    Test_MMX(640,480,24,8);
    system("pause");
	return 0;
}

