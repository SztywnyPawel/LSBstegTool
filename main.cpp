#include <iostream>
#include <bitset>
#include <algorithm>

#include <stdio.h>
#include <stdalign.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <bits/stdc++.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

inline const uint64_t hash_64_fnv1a(const void* key, const uint64_t len) {
    
    const char* data = (char*)key;
    uint64_t hash = 0xcbf29ce484222325;
    uint64_t prime = 0x100000001b3;
    
    for(int i = 0; i < len; ++i) {
        uint8_t value = data[i];
        hash = hash ^ value;
        hash *= prime;
    }
    
    return hash;

}
void printByteBinary(const char& byte)
{
    std::cout << std::bitset<8>(byte)<<std::endl;
}
void lsbModify(unsigned char *pixChan , const bool& messBit){
    if(messBit) *pixChan |=1;
    else *pixChan &= ~1;
}
std::vector<unsigned int> hashVec(size_t hash){
    std::vector<unsigned int> hashChars;
    while (true)
    {
        if(hash==0)return hashChars;
        size_t lastChar = (hash%10)+1;
        hashChars.push_back(lastChar);
        hash= (hash-(hash%10))/10;
    }
    return {};
}
std::vector<bool> strToBit(const std::string& message){
    std::vector<bool> bitVec;
    bitVec.reserve(message.size()*8);
    for(int i=0;i<message.size();++i){
        for(int j = 7; j>=0;--j)bitVec.push_back((message[i]>>j) & 1);
    }
    return bitVec;
}
void imgMessWrite(unsigned char *img,std::string password,int pixelsAmmount,std::vector<bool>boolVec){
    for (int i = 0; i < 8; i++)
    {
        boolVec.push_back(false);
    }
    uint64_t hashResult = hash_64_fnv1a(password.c_str() , password.length()+1);
    std::vector<unsigned int> hashChars = hashVec(hashResult);
    int hashVecLength = hashChars.size();
    int hashVecSum=0;
    int actualChar = 0;
    int binMessPos = 0;
    int binMessLength = boolVec.size();
    for(int i= 0; i<pixelsAmmount;i+=hashChars[actualChar],actualChar++,++binMessPos){ 
        if(binMessPos==binMessLength)return;
        if(actualChar==hashVecLength)actualChar=0;

        lsbModify(img+i,boolVec[binMessPos]);
    }
}
std::string imgMessRead(unsigned char *img,std::string password,int pixelsAmmount){
    uint64_t hashResult = hash_64_fnv1a(password.c_str() , password.length()+1);
    std::vector<unsigned int> hashChars = hashVec(hashResult);
    int hashVecLength = hashChars.size();
    int nullCount = 0;
    std::string found;
    int binToDecBuff = 0;
    std::string result;
    for(int i= 0 , actualChar = 0 , loopCount = 1; i<pixelsAmmount;i+=hashChars[actualChar],actualChar++ , loopCount++){
        if(actualChar==hashVecLength)actualChar=0;
        bool  actuallLSB = std::bitset<8>(img[i])[0];
        nullCount++;
        if(actuallLSB)nullCount = 0;
        bool modulo8 = loopCount%8==0;
        if(nullCount>=8 && modulo8)return result;
        binToDecBuff = (binToDecBuff<<1)|(actuallLSB?1:0);
        if(modulo8){result+=binToDecBuff;binToDecBuff=0;}
    }
    return result;
}

void viewPixelsBinary(unsigned char *img , int width , int height , int channels){
    std::cerr << "Ammount of bytes:"<<width*height*channels<<"\n";
//how much pixels to show
    int show_pixel_ammount;
    std::cin >> show_pixel_ammount;
    if(show_pixel_ammount > width*height){
        std::cout<<"Error too big number";
        show_pixel_ammount = width*height;
        //idk
        // return 1;
    }

    std::cout << "Width: " << width
            << " Height: " << height
            << " Channels: " << channels << "\n";

    int totalPixels = width * height;
    int pixelsToPrint = std::min(show_pixel_ammount, totalPixels);

    for (int i = 0; i < pixelsToPrint; ++i)
    {
        unsigned char* pixel = img + (i * channels);

        std::cout << "Pixel " << i+1 << ": ";

        for (int j = 0; j < channels; ++j)
        {
            printByteBinary(pixel[j]);
            std::cout << " ";
        }

        std::cout << "\n";
    }
}

int main(int argc, char* argv[])
{
    int option;
    unsigned int flagsCount = 0;
    bool eFlag = false;
    bool dFlag = false;
    bool vFlag = false;
    bool oFlag = false;
    std::string passwd;
    std::string outImageName;
    while ((option = getopt(argc , argv , "edvo:")) !=-1)
    {
        switch (option)
        {
            case 'e':
                eFlag = true;
                flagsCount++;
                break;  
            case 'd':
                dFlag = true;
                flagsCount++;
                break;
            case 'v':
                vFlag = true;
                flagsCount++;
                break;
            case 'o':
                 outImageName = optarg;
                 oFlag = true;
                 break;
        }
    }

    if(flagsCount!=1){
        std::cerr<<"Flags error\n";
        return 1;
    }

    std::string filePath;
    if (optind<argc)filePath = argv[optind];
    if (filePath.empty()){
        std::cerr<<"No path set\n";
        return 1;
    }

    int width , height , channels;
    unsigned char *img = stbi_load(filePath.c_str(), &width , &height , &channels , 0);
    if (!img)
    {
        std::cerr << "Error loading image\n";
        return 1;
    }

    if(vFlag){
        viewPixelsBinary(img , width , height , channels);
    }
    if(dFlag){
        passwd = getpass("Enter password: ");
        std::cout<<imgMessRead(img,passwd,width*height*channels);
    }
    if(eFlag){
        std::string mess(
            std::istreambuf_iterator<char>(std::cin),
            std::istreambuf_iterator<char>()
        );
        std::cout<<"message:"<<mess<<"\n";
        passwd = getpass("Enter password: ");
        imgMessWrite(img,passwd,width*height*channels,strToBit(mess));
        if(!oFlag)outImageName = filePath;
        //there gonna be stuff like checking if it's .png
        stbi_write_png(outImageName.c_str() , width , height , channels , img, width*channels);
    }
    std::cout<<"passwd: "<<passwd;

    stbi_image_free(img);
    return 0;
} 