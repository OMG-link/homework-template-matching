#include <cstdio>

#include "data.cpp"


template<int H,int W>
void print(unsigned char c[H][W],const char* path){
	FILE* fp = fopen(path,"w");
	fprintf(fp,"%d %d\n",H,W);
	for(int i=0;i<H;i++){
		for(int j=0;j<W;j++){
			fprintf(fp,"%d%c",c[i][j]," \n"[j==W-1]);
		}
	}
	fclose(fp);
}

int main(){
	print<64,64>(cImageTemplate,"template.txt");
	print<256,256>(cImageTarget,"image.txt");
}
