/*
Corey De Vany
ColorConvert.cpp
CS-3150
11/18/2020
*/

/**
 * ColorConvert.cpp
 * This program takes two arguments:
 *      (1) The file name to convert
 *      (2) An integer to represent the desired conversion method. 
 *      Here are the corresponding values for the conversion methods:
 *          1: Lightness Conversion Method
 *          2: Average Conversion Method
 *          3: Luminosity Conversion Method
 * 
 * The output file name will automatically be appended with the proper conversion method,
 * and will output into the directory this program is ran from.
 * 
 * EZ GRADING RUBRIC:
 *  Confirm it's a valid BMP:           Line 153
 *  Validate based on header size:      Line 155
 *  Name the created file correctly:    Method on line 75, implemented on 201-218
 *  Command line input handled:         Lines 130-132
 *  Lightness formula definition:       Line 85
 *  Average formula definition:         Line 99
 *  Luminosity Formula definition:      Line 108
 *  Formula implementations:            Lines 169-197
 *  Created final output:               Lines 221-222
 */ 

#include <iostream>
#include <iomanip>
#include <fstream>
#include <array>
#include <string>

using namespace std;


/******** TYPE DEFINITIONS ********/

typedef unsigned char byte;

#pragma pack(push, 1)					    //just to make sure these things are stored without padding.

	/* This struct will save the first 14 bytes in the BMP header. */
	struct BMP_Header {
		uint16_t headerCheck{0};			//these two bytes store the header extracted from the file.
		uint32_t fileSize{0};				//the next 4 bytes store the file size.
		uint16_t reserved1{0};			    //reserved, actual value depends on what created the image
		uint16_t reserved2{0};			    //reserved, same as above
		uint32_t startAddress{0};			//starting address of the byte were the bitmap image data can be found.
	};

    /* For now, this program only handles 24 bit bitmaps so a Pixel has 3 bytes. */
    struct Pixel {
        byte red;
        byte green;
        byte blue;
    };

#pragma pack(pop)


/******** HELPER FUNCTIONS ********/

//retrieves the decimal integer value from a 4 byte little endian hex value
uint32_t uint32FromHex(uint32_t &fourBytes){
	byte* value { (byte*)&fourBytes };
	return (value[0] | value[1] << 8 | value[2] << 16 | value[3] << 24);
}

//strips the raw file name. Found between the last / and . 
string stripFileName(string &name){
    string newName = name.substr(name.find_last_of("/") + 1);
    newName = newName.substr(0, newName.find_last_of("."));
    return newName;
}


/******** COLOR CONVERSION FUNCTIONS ********/

//Lightness Method: (max(R,G,B) + min(R,G,B)) / 2
 byte lightnessConversion(Pixel &p) {
    byte b{0};
    uint8_t max, min;
    //find max
    max = (p.red > p.green) ? (p.red > p.blue ? p.red : p.blue) : (p.green > p.blue ? p.green : p.blue);
    //find min
    min = p.blue < (p.red < p.green ? p.red : p.green) ? p.blue : (p.red < p.green ? p.red:p.green);
    //avoid integer overflow
    int sum = (max + min)/2;
    b += sum;
    return b;
 }

//Average Method: (R+G+B)/3
byte averageConversion(Pixel &p) {
    byte b{0};
    //avoid overflow
    int average = (p.red + p.green + p.blue) / 3;
    b += average;
    return b;
}

//Luminosity Method: .21*R + .72*G +.07*B
byte luminosityConversion(Pixel &p) {
    double r = .21*p.red;
    double g = .72*p.green;
    double b = .07*p.blue;
    uint8_t sum = r + g + b;
    return (byte)sum;
}


int main(int argc, char* argv[]) {


/******** VALIDATE THE ARGUMENTS********/

    //check for proper arg count
    if (argc != 3) { 
        cout << "Please enter a file name to read, and a digit [1/2/3] for the conversion method. " << endl; 
        cout << "Option 1: Lightness Conversion Option 2: Average Conversion Option 3: Luminosity Conversion. " << endl;
        return -1; 
    }
    
    //save args in memory
    string fileName = string(argv[1]);
    int conversionMethod = atoi(argv[2]);
    string rawName = stripFileName(fileName);

    //validate conversionMethod
    if(conversionMethod <= 0 || conversionMethod >= 5) { throw runtime_error("Invalid conversion method. Please enter 1, 2, or 3. "); }

    
/******** READ THE FILE INTO MEMORY AND VALIDATE HEADER INFO ********/

    //open the input stream
    ifstream inp( fileName, ifstream::binary);

    //get the header information 
    inp.seekg(0);
    BMP_Header header{};
    inp.read((char*)&header, sizeof(header));

    //double check the file size by iterating to the end and counting the bytes
    inp.seekg(0, ios::end);
	int size = inp.tellg();

    //validate the header: both file signature and size
    if(header.headerCheck != 0x4D42) { throw runtime_error("BMP header check failed!"); }
    uint32_t length = uint32FromHex(header.fileSize);
    if(length != size) { throw runtime_error("BMP size check failed!"); }

    //create a buffer of the proper size
    byte* buffer = new byte[size];

    //read the file into the buffer
    inp.seekg(0);
    inp.read((char*)buffer, size);


/******** CONVERT COLORS TO B&W ********/

    //iterate through the image data.
    for(int i = uint32FromHex(header.startAddress); i < size; i++) {
        int j = i;
        byte newValue;

        //create a temp pixel with the next 3 bytes
        Pixel pixel{buffer[i], buffer[++i], buffer[++i]};

        //convert the pixel values to a single shade of grey.
        switch(conversionMethod){
            case 1:
                newValue = lightnessConversion(pixel);
                break;

            case 2:
                newValue = averageConversion(pixel);
                break;
            
            case 3:
                newValue = luminosityConversion(pixel);
                break;

            default:
                break;
        }
        //write back out to the buffer
        while(j <= i) {
            buffer[j++] = newValue;
        }
    }


/******** MAKE THE NEW FILE NAME AND WRITE OUT THE BUFFER ********/

    string newFileName;
    //modify the new file name with the appropriate method used
    switch(conversionMethod) {
        case 1:
            newFileName = rawName.append("-LightnessConversion.bmp");
            break;

        case 2:
            newFileName = rawName.append("-AverageConversion.bmp");
            break;

        case 3:
            newFileName = rawName.append("-LuminosityConversion.bmp");
            break;

        default:
            break;
    }

    //create the output stream and write the file
    ofstream outp(newFileName, ofstream::binary);
    outp.write((char*)buffer, size);

/******** CLEAN UP ********/

    inp.close();
    outp.close();
}