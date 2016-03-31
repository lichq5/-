#include "CImg.h"
#define PI 3.141592653

using namespace cimg_library;

int double2int(double num) {
	double temp = floor(num) + 0.5;
	return num >= temp ? ceil(num) : floor(num);
}

void rotated_img(CImg<unsigned char> img, double angel) {
	int w = img.width();
	int h = img.height();
	int z = img.depth();
	int v = img.spectrum();
	int d = double2int(sqrt(w*w + h*h));
	int r = d / 2;
	int m = h / 2;
	int n = w / 2;
	double rad = angel / 360 * 2 * PI;
	CImg<unsigned char> rotateImg(d, d, z, v);
	rotateImg.fill(0);
	cimg_forY(img, y) {
		cimg_forX(img, x) {
			int newy = double2int(abs((y - m)*cos(rad) - (x - n)*sin(rad) + r));
			int newx = double2int(abs((y - m)*sin(rad) + (x - n)*cos(rad) + r));
			cimg_forC(img, c) {
				rotateImg(newx, newy, z, c-1) = img(x, y, z, c-1);
			}
		}
	}
	
	rotateImg.display("rotated");
}

int main() {
	CImg<unsigned char> image("1.bmp");
	rotated_img(image, 150);
	return 0;
}