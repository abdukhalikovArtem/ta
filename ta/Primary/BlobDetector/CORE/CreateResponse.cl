inline double GetExternPixel(short y, short x, __global double* data, short img_width, short img_height) {
	if ((x >= 0) && (x < img_width) && (y >= 0) && (y < img_height))
		return data[y * img_width + x];

	bool hit_corner_left_top = (x < 0) && (y < 0);
	bool hit_corner_right_top = (x >= img_width) && (y < 0);
	bool hit_corner_left_bot = (x < 0) && (y >= img_height);
	bool hit_corner_right_bot = (x >= img_width) && (y >= img_height);

	bool hit_border_top = (y < 0);
	bool hit_border_bot = (y >= img_height);
	bool hit_border_left = (x < 0);
	bool hit_border_right = (x >= img_width);

	if (hit_corner_left_top)
		return data[0 * img_width + 0];
	if (hit_corner_right_top)
		return data[img_width - 1];
	if (hit_corner_left_bot)
		return data[(img_height - 1) * img_width + 0];
	if (hit_corner_right_bot)
		return data[(img_height - 1) * img_width + img_width - 1];

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



__kernel void Core(__constant char* _sigmaNumber, __global double* buffer, __constant double* row, __constant short* _filterSize, __global double* response)
{
	const short i     = get_global_id(0);
    const short j     = get_global_id(1);
    const short imgWidth  = get_global_size(1);
	const short imgHeight  = get_global_size(0);
    const int id = i * imgWidth + j;
	
	short filterSize = _filterSize[0];
	char sigmaNumber = _sigmaNumber[0];
	int diff = (filterSize - 1) / 2;
	double acc = 0;

	for (int m = 0; m < filterSize; m++){ 
		int abs_slw_y = m - diff;
		int abs_slw_x = 0;
		int x = j + abs_slw_x;
		int y = i + abs_slw_y;
					
		acc += GetExternPixel(y, x, buffer, imgWidth, imgHeight) * row[sigmaNumber*filterSize + m];
	}
	
	response[id] = (sigmaNumber == 0)? acc : response[id] + acc;
}
