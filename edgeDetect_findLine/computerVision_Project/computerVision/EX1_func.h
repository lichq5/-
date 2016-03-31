
int round(double x) {
	double temp = floor(x)+0.5;
	return x >= temp? ceil(x) : floor(x);
}

/*
 *反向映射回原图
 *2次线性内插法缩放
 */
CImg<unsigned char> imresize(CImg<unsigned char> img, double times) {
	int w = img.width();
	int h = img.height();
	int z = img.depth();
	int v = img.spectrum();

	CImg<unsigned char> tempImg(w+2,h+2,z,v);  /* 边界填充一个像素 */
	int newWidth, newHight;
	try {

		newWidth = round(w*times);
		newHight = round(h*times);
		
		if (newWidth < 1 || newHight < 1) {
		   throw newWidth+newHight;
		}
	}
	catch(int) {
		cout << "----------ERROR: times is to small------------" << endl;
	}
	CImg<unsigned char> resizeImg(newWidth,newHight,z,v);
	tempImg.fill(0);
	cimg_forXYZC(img, x,y,z,v) {
		tempImg(x+1, y+1, z, v) = img(x,y,z,v);
	}
	double src_x;
	double src_y;

	/* 表示对应原图坐标的小数部分， 在计算权重时会用到 */
	double dis_a;
	double dis_b;
	int src_i, src_j;
	cimg_forXYZC(resizeImg, x,y,z,v) {
		src_x = x/times;
		src_y = y/times;
		src_i = int(floor(src_x));
		src_j = int(floor(src_y));
		dis_a = src_x-src_i;
		dis_b = src_y-src_j;
		resizeImg(x, y, z, v) = (1-dis_a)*(1-dis_b)*tempImg(src_i,src_j,z,v) + 
			(1-dis_a)*dis_b*tempImg(src_i,src_j+1,z,v) + dis_a*(1-dis_b)*tempImg(src_i+1,src_j,z,v) + 
			dis_a*dis_b*tempImg(src_i+1,src_j+1,z,v);

	}
	//resizeImg.display("resize image");
	return resizeImg;
}

/*
 *2次线性内插法旋转
 */
CImg<unsigned char> rotate_img(CImg<unsigned char> img, double angel) {
	int w = img.width();
	int h = img.height();
	int z = img.depth();
	int v = img.spectrum();

	/* 计算旋转后的图像需要的宽高， 我用原图的对角线替代 */
	int d = round(sqrt(w*w+h*h));
	int r = d/2;

	/* 求原图的中心坐标 */
	int m = h/2;
	int n = w/2;
	double rad = angel/360*2*PI;
	CImg<unsigned char> rotateImg(d,d,z,v);
	rotateImg.fill(0);
	double cosV = cos(rad);
	double sinV = sin(rad);
	cimg_forY(rotateImg,y) {
		cimg_forX(rotateImg,x) {
			double src_x = n+(x-r)*cosV-(y-r)*sinV;
			double src_y = m+(y-r)*cosV+(x-r)*sinV;
			int int_srcx = floor(src_x);
			int int_srcy = floor(src_y);
			double i = src_x-int_srcx;
			double j = src_y-int_srcy;    /* 如果反向映射的点在原图的范围中， 那么赋值*/
			if (int_srcx < w && int_srcx >= 0 && int_srcy < h && int_srcy >= 0) { 
			cimg_forC(rotateImg, c) {
				rotateImg(x,y,z,c-1) = (1-i)*(1-j)*img(int_srcx,int_srcy,z,c-1)+
					(1-i)*j*img(int_srcx,int_srcy+1,z,c-1)+i*(1-j)*img(int_srcx+1,int_srcy,z,c-1)+
					i*j*img(int_srcx+1,int_srcy+1,z,c-1);
			}
			}
		}
	}
	//rotateImg.display("1");
	return rotateImg;
}

/*
 *最邻近内插法旋转
 */
CImg<unsigned char> rotate_img2(CImg<unsigned char> img, double angel) {
	int w = img.width();
	int h = img.height();
	int z = img.depth();
	int v = img.spectrum();

	/* 计算旋转后的图像需要的宽高， 我用原图的对角线替代 */
	int d = round(sqrt(w*w+h*h));
	int r = d/2;

	/* 求原图的中心坐标 */
	int m = h/2;
	int n = w/2;
	double rad = angel/360*2*PI;
	CImg<unsigned char> rotateImg(d,d,z,v);
	rotateImg.fill(0);
	double cosV = cos(rad);
	double sinV = sin(rad);
	cimg_forY(img, y) {
		cimg_forX(img, x) {
			int newy = round(abs((y-m)*cosV-(x-n)*sinV+r));
			int newx = round(abs((y-m)*sinV+(x-n)*cosV+r));
			cimg_forC(img, c) {
				rotateImg(newx,newy,z,c-1) = img(x,y,z,c-1);
			}
		}
	}
	
	/* 使用最近邻内插法填补黑点 */
	int right_most = 0;
	cimg_forY(rotateImg, yy) {
		cimg_forX(rotateImg, xx) { /* 找到每行的最右的边界点 */
			if (rotateImg(xx,yy,z,-1) != 0||rotateImg(xx,yy,z,0) != 0||rotateImg(xx,yy,z,1) != 0)
				right_most = xx;
		}
		/* 临时变量，记录要内插的邻近的像素值 */
		int dim = 0;
		int dim0 = 0;
		int dim1 = 0;

		for(int xx = 0; xx < right_most; xx++) {
			if (rotateImg(xx,yy,z,-1) == 0&&rotateImg(xx,yy,z,0) == 0&&rotateImg(xx,yy,z,1) == 0) {
				rotateImg(xx,yy,z,-1) = dim;
				rotateImg(xx,yy,z,0) = dim0;
				rotateImg(xx,yy,z,1) = dim1;
			} else {
				dim = rotateImg(xx, yy, z, -1);
				dim0 =rotateImg(xx, yy, z, 0);
				dim1 = rotateImg(xx, yy, z, 1);
			}
		}
	}
	//rotateImg.display("2");
	return rotateImg;
}

/*
 *画矩形，传的参数包括要旋转的图像，宽和高，还有一个角点坐标
 */

CImg<unsigned char> draw_rectangle(CImg<unsigned char> graphics, double width, double height, double x0, double y0) {
	const unsigned char color[] = { 255,255,255 };
	int x1 = x0 + width;
	int y1 = y0;
	int x2 = x0 + width;
	int y2 = y0 + height;
	int x3 = x0;
	int y3 = y0 + height;
	graphics.draw_line(x0, y0, x1, y1, color);
	graphics.draw_line(x1, y1, x2, y2, color);
	graphics.draw_line(x2, y2, x3, y3, color);
	graphics.draw_line(x3, y3, x0, y0, color);
	return graphics;
}

/*
 *画圆，传的参数包括要旋转的图像，半径，和圆心坐标
 */

CImg<unsigned char> draw_circular(CImg<unsigned char> graphics, double radius, double x0, double y0) {
	const unsigned char color[] = { 255,255,255 };
	double x;
	double y;
	double doublePI = 2*PI;
	double alpha;
	for (double i = 0; i <= 360; i += 0.001) {
		alpha = i/360*doublePI;
		x = x0 + radius*cos(alpha);
		y = y0 + radius*sin(alpha);
		graphics.draw_point(x, y, color);
	}
	return graphics;
}

/*
 *画三角形传的是一个定点和交与定点的两边长，夹角的大小
 */

CImg<unsigned char> draw_triangle(CImg<unsigned char> graphics, double side1, double side2, double x0, double y0, double angle) {
	const unsigned char color[] = { 255,255,255 };
	double x1, y1, x2, y2;
	x1 = x0 + side1;
	y1 = y0;
	x2 = x0 + side2*cos(angle/360*2*PI);
	y2 = y0 + side2*sin(angle/360*2*PI);
	graphics.draw_line(x0, y0, x1, y1, color);
	graphics.draw_line(x1, y1, x2, y2, color);
	graphics.draw_line(x2, y2, x0, y0, color);
	return graphics;
}
