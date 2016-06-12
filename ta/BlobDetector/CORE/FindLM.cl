
inline double GetExternPixel(short y, short x, __global double* data, short img_width, short img_height) {
	if ((x >= 0) && (x < img_width) && (y >= 0) && (y < img_height))
		return data[y * img_width + x];

	bool hit_corner_left_top = (x < 0) && (y < 0);
	bool hit_corner_right_top = (x >= img_width) && (y < 0);
	bool hit_corner_left_bot = (x < 0) && (y >= img_height);
	bool hit_corner_right_bot = (x >= img_width) && (y >= img_height);

	if (hit_corner_left_top)
		return data[0 * img_width + 0];
	if (hit_corner_right_top)
		return data[img_width - 1];
	if (hit_corner_left_bot)
		return data[(img_height - 1) * img_width + 0];
	if (hit_corner_right_bot)
		return data[(img_height - 1) * img_width + img_width - 1];

	bool hit_border_top = (y < 0);
	bool hit_border_bot = (y >= img_height);
	bool hit_border_left = (x < 0);
	bool hit_border_right = (x >= img_width);

	if (hit_border_top)
		return data[0 + x];
	if (hit_border_bot)
		return data[(img_height - 1) * img_width + x];
	if (hit_border_left)
		return data[y * img_width + 0];
	if (hit_border_right)
		return data[y * img_width + img_width - 1];

	return data[y * img_width + x];
};

__kernel void Core( __global float* response,__global char* lm)
{
	const short i     = get_global_id(0);
    const short j     = get_global_id(1);
    const short imgWidth  = get_global_size(1);
	const short imgHeight  = get_global_size(0);
    const int id = i * imgWidth + j;
	
	char maxPixelCounter = 0;
	char minPixelCounter = 0;
	
	int diff = 1;
	
	float center = GetExternPixel(i, j, response, imgWidth, imgHeight);
	for (char m = -diff; m <= diff; ++m){ 
		for (char n = -diff; n <= diff; ++n){
			short imgPos_i = i + m;
			short imgPos_j = j + n;
			float buffer =  GetExternPixel(imgPos_i, imgPos_j, response, imgWidth, imgHeight);

			if ((m == 0) && (n == 0)) 
				continue;

			maxPixelCounter = (((center) - (buffer)) > -0.0)? maxPixelCounter + 1 : maxPixelCounter;
			minPixelCounter = (((center) - (buffer)) < 0.0)? minPixelCounter + 1 : minPixelCounter;
		}
	}
	
	bool maxPixel = maxPixelCounter == 8;
	bool minPixel = minPixelCounter == 8;

	lm[id] = (maxPixel)? 255 : (minPixel)? 0 : 127;
}
