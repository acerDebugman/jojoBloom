#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "../jojo/jojo.h"
#include <iostream>
#include <ctime>
#include <cstdlib>

using namespace std;

class GenerateData {
public :
	GenerateData(const char *fileName, int wordLen, int count){
		strcpy(this->fileName, fileName);
		this->wordLen = wordLen;
		this->itemCount = count;
	}
	~GenerateData(){

	}

	void generateData(){
		int fd;
		if((fd = open(fileName, O_RDWR | O_CREAT | O_TRUNC)) < 0)
			JojoUtil::err_sys("open file error!");

		std::srand((unsigned)std::time(NULL));
		for(int i = 0; i < itemCount; i++){
			char *word = (char *)malloc(sizeof(char) * (wordLen + 2));
			gen_random(word, wordLen);

			if(write(fd, word, wordLen + 1) < wordLen + 1)
				JojoUtil::err_sys("write error!");
		}
		close(fd);
	}

	bool verifyItem(){
		//how to do this efficiently ??
		long begin = JojoUtil::nowMicros();



		long end = JojoUtil::nowMicros();
		printf("time cost : %l\n",  (long)(end - begin));
		return true;
	}

protected:
	void gen_random(char *s, int len){
		int modnum = strlen(alphaNum);

		for(int i = 0; i < len; i++){
			int r = std::rand();
			s[i] = alphaNum[r % (modnum - 1)];
		}
		s[len] = '\n';
		s[len + 1] = 0;
	}

private:
	int itemCount;
	int wordLen;
	char fileName[200];
	const char *alphaNum = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
//	const char *alphaNum = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
//	const char *alphaNum = "abcdefghijklmnopqrstuvwxyz";
};

int main(int argc, char ** argv){
	if(argc < 2)
		JojoUtil::err_sys("usage: %s item_limit word_length", argv[0]);

	GenerateData gd("/root/data.word", atoi(argv[1]), atoi(argv[2]));
	gd.generateData();
}

