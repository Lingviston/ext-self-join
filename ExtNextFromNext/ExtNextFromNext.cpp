// ExtNextFromNext.cpp: определяет точку входа для консольного приложения.
//


#include "stdafx.h"
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <string>
#include <algorithm> 

typedef unsigned int UINT;

static const int buffer_size = 512 * 512;
static UINT *buffer = new UINT[buffer_size / sizeof(UINT)];

int compare1(const void * a, const void * b)
{
	if (*(UINT*)a > *(UINT*)b)
	{
		return 1;
	}
	else if (*(UINT*)a < *(UINT*)b)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int compare2(const void * a, const void * b)
{
	if (*((UINT*)a + 1) > *((UINT*)b + 1))
	{
		return 1;
	}
	else if (*((UINT*)a + 1) < *((UINT*)b + 1))
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void ext_sort(FILE *input, FILE *output, FILE *temp, int element_count, size_t element_size, int(*compare)(const void*, const void*), bool write_size) {
	const int buffer_lenght = buffer_size / element_size * element_count;

	int count;
	fread(&count, sizeof(UINT), 1, input);
	if (write_size) {
		fwrite(&count, sizeof(UINT), 1, output);
	}

	// Filling the temp file
	const int blocks_count = element_count * count / buffer_lenght + (((element_count * count) % buffer_lenght) ? 1 : 0);
	int* block_lenghts = new int[blocks_count];
	int count_left = count * element_count;
	for (int i = 0; i < blocks_count; ++i, count_left -= buffer_lenght) {
		block_lenghts[i] = std::min(buffer_lenght, count_left);

		fread(buffer, element_size / element_count, block_lenghts[i], input);

		qsort(buffer, block_lenghts[i] / element_count, element_size, compare);

		fwrite(buffer, element_size / element_count, block_lenghts[i], temp);
	}

	const int block_size = buffer_size / (blocks_count + 1);
	const int block_lenght = block_size / element_size * element_count;

	UINT** buffers = new UINT*[blocks_count];
	int* block_positions = new int[blocks_count];
	for (int i = 0; i < blocks_count; ++i) {
		block_positions[i] = 0;
		buffers[i] = buffer + i * block_lenght;
		fseek(temp, (element_size / element_count) * (i * buffer_lenght), SEEK_SET);
		fread(buffers[i], element_size / element_count, block_lenght, temp);
	}

	UINT* write_buffer = buffer + blocks_count * block_lenght;
	int write_buffer_position = 0;
	for (int i = 0; i < count; ++i) {
		UINT* min_element = nullptr;
		int min_element_block = -1;

		// Searching for min element value and block number
		for (int j = 0; j < blocks_count; ++j) {
			if (block_positions[j] == block_lenghts[j]) {
				continue;
			}
			else if (min_element == nullptr) {
				min_element_block = j;
				min_element = &buffers[j][block_positions[j] % block_lenght];
			}
			else if (compare((void*)&buffers[j][block_positions[j] % block_lenght], min_element) < 0)
			{
				min_element = &buffers[j][block_positions[j] % block_lenght];
				min_element_block = j;
			}
		}

		// Put min element into write buffer
		for (int j = 0; j < element_count; ++j) {
			write_buffer[write_buffer_position + j] = min_element[j];
		}
		write_buffer_position += element_count;

		// Flush write buffer if necessary
		if (write_buffer_position == block_lenght) {
			fwrite(write_buffer, element_size / element_count, write_buffer_position, output);
			write_buffer_position = 0;
		}

		// Read new buffer for the min element if neccessary
		block_positions[min_element_block] += element_count;
		if (block_positions[min_element_block] < block_lenghts[min_element_block] &&
			block_positions[min_element_block] % block_lenght == 0) {
			fseek(temp, (element_size / element_count) * (min_element_block * buffer_lenght + block_positions[min_element_block]), SEEK_SET);
			fread(buffers[min_element_block], element_size / element_count, std::min(block_lenghts[min_element_block] - block_positions[min_element_block], block_lenght), temp);
		}
	}

	// Flushing what's left
	if (write_buffer_position != 0) {
		fwrite(write_buffer, element_size / element_count, write_buffer_position, output);
		write_buffer_position = 0;
	}

	delete[] block_positions;
	delete[] block_lenghts;
}

void create_test_input() {
	FILE *output = NULL;

	errno_t err = 0;
	err = fopen_s(&output, "input.bin", "wb");
	if (err != 0) {
		printf("Error opening test file\n");
		return;
	}

	int count = 9;
	fwrite(&count, sizeof(int), 1, output);

	UINT temp = 4;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 3;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 9;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 2;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 3;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 5;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 6;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 7;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 1;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 8;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 7;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 1;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 2;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 6;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 5;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 9;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 8;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 4;
	fwrite(&temp, sizeof(UINT), 1, output);

	fclose(output);
}

void display_result(const char* filename, int element_count, int count) {
	errno_t err = 0;

	FILE *result = NULL;
	err = fopen_s(&result, filename, "rb");
	if (err != 0) {
		printf("Error opening result file\n");
		return;
	}

	if (count == 0) {
		fread(&count, sizeof(int), 1, result);
		std::cout << count << '\n';
	}

	UINT *element = new UINT[element_count];
	for (int i = 0; i < count; ++i) {

		fread(element, sizeof(UINT), element_count, result);

		for (int j = 0; j < element_count; ++j) {
			std::cout << (UINT)element[j] << " ";
		}
		std::cout << '\n';
	}

	delete[] element;
	fclose(result);
}

void join(FILE *input1, FILE *input2, FILE *output) {
	int count1;
	fread(&count1, sizeof(int), 1, input1);

	int count2;
	fread(&count2, sizeof(int), 1, input2);

	if (count1 != count2) {
		printf("Can't merge lists of different lenght\n");
		return;
	}

	const int base_buffer_lenght = buffer_size / 7 / sizeof(UINT);

	UINT* input_buffer1 = buffer;
	UINT* input_buffer2 = buffer + base_buffer_lenght * 2;
	UINT* output_buffer = buffer + base_buffer_lenght * 4;

	int count = count1;

	fwrite(&count, sizeof(int), 1, output);
	for (int i = 0; i < count; i += base_buffer_lenght) {
		fread(input_buffer1, sizeof(UINT), base_buffer_lenght * 2, input1);
		fread(input_buffer2, sizeof(UINT), base_buffer_lenght * 2, input2);

		for (int j = 0; j < std::min(base_buffer_lenght, count - i); ++j) {
			output_buffer[j * 3] = input_buffer2[j * 2];
			output_buffer[j * 3 + 1] = input_buffer2[j * 2 + 1];
			output_buffer[j * 3 + 2] = input_buffer1[j * 2 + 1];
		}

		fwrite(output_buffer, sizeof(UINT), std::min((count - i) * 3, base_buffer_lenght * 3), output);
	}
}

int main()
{
	create_test_input();
	display_result("input.bin", 2, 0);

	const auto startTime = std::clock();

	errno_t err = 0;
	FILE *input = NULL;
	err = fopen_s(&input, "input.bin", "r+b");
	if (err != 0) {
		printf("Error opening input file\n");
		return 0;
	}

	FILE *output = NULL;
	err = fopen_s(&output, "output.bin", "w+b");
	if (err != 0) {
		printf("Error opening output file\n");
		return 0;
	}

	FILE *temp = NULL;
	err = fopen_s(&temp, "temp.bin", "w+b");
	if (err != 0) {
		printf("Error opening temp file\n");
		return 0;
	}

	ext_sort(input, output, temp, 2, 2 * sizeof(UINT), compare1, true);
	/*fclose(output);
	display_result("output.bin", 2, 0);*/

	rewind(input);
	rewind(output);
	rewind(temp);
	ext_sort(output, input, temp, 2, 2 * sizeof(UINT), compare2, true);
	/*fclose(input);
	display_result("input.bin", 2, 0);*/

	rewind(input);
	rewind(output);
	rewind(temp);
	join(output, input, temp);
	/*fclose(temp);
	display_result("temp.bin", 3, 0);*/

	rewind(input);
	rewind(output);
	rewind(temp);
	ext_sort(temp, output, input, 3, 3 * sizeof(UINT), compare1, false);
	fclose(temp);
	fclose(input);
	fclose(output);


	// Calculating execution time
	const auto endTime = std::clock();
	double time = double(endTime - startTime) / CLOCKS_PER_SEC;

	display_result("output.bin", 3, 9);
	std::cerr << "Time: " << time << '\n';

	return 0;
}
