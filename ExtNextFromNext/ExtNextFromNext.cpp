// ExtNextFromNext.cpp: определяет точку входа для консольного приложения.
//


#include "stdafx.h"
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <string>
#include <algorithm> 

typedef unsigned int UINT;

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

// element size should be  8 or 12, while count 2 or 3
void ext_sort(const char* input_filename, const char* output_filename, int element_count, size_t element_size, int(*compare)(const void*, const void*), bool write_size) {
	// Opening input file
	FILE *input = NULL;
	errno_t err = 0;
	err = fopen_s(&input, input_filename, "rb");
	if (err != 0) {
		printf("Error opening input file\n");
		return;
	}

	// Reading array size
	int count;
	fread(&count, sizeof(int), 1, input);

	// Creating temp files
	const int buffer_size = 256 * 384;
	const int buffer_lenght = buffer_size / element_size * element_count;
	const int temp_files_count = element_count * count / buffer_lenght + (((element_count * count) % buffer_lenght) ? 1 : 0);

	UINT *buffer = new UINT[buffer_lenght];

	FILE** temp_files = new FILE*[temp_files_count];
	int* temp_file_lenghts = new int[temp_files_count];

	std::string ext = ".bin";
	int count_left = count * element_count;
	for (int i = 0; i < temp_files_count; ++i, count_left -= buffer_lenght) {
		err = fopen_s(&temp_files[i], (std::to_string(i) + ext).c_str(), "w+b");
		if (err != 0) {
			printf("Error opening temp file\n");
			return;
		}

		temp_file_lenghts[i] = std::min(buffer_lenght, count_left);

		fread(buffer, element_size / element_count, temp_file_lenghts[i], input);

		qsort(buffer, temp_file_lenghts[i] / element_count, element_size, compare);

		fwrite(buffer, element_size / element_count, temp_file_lenghts[i], temp_files[i]);
	}

	delete[] buffer;
	fclose(input);

	// Resetting temp files' pointers
	const int block_size = buffer_size / (temp_files_count + 1);
	const int block_lenght = block_size / element_size * element_count;
	UINT** buffers = new UINT*[temp_files_count];
	int* temp_file_positions = new int[temp_files_count];
	for (int i = 0; i < temp_files_count; ++i) {
		temp_file_positions[i] = 0;

		rewind(temp_files[i]);

		buffers[i] = new UINT[block_lenght];
		fread(buffers[i], element_size / element_count, block_lenght, temp_files[i]);
	}


	// Creating output file
	FILE *output = NULL;
	err = fopen_s(&output, output_filename, "wb");
	if (err != 0) {
		printf("Error opening output file\n");
		return;
	}
	if (write_size) {
		fwrite(&count, sizeof(int), 1, output);
	}

	// Merging sorted files
	UINT* write_buffer = new UINT[block_lenght];
	int write_buffer_position = 0;
	for (int i = 0; i < count; ++i) {
		UINT* min_element = nullptr;
		int min_element_block = -1;

		// Searching for min element value and block number
		for (int j = 0; j < temp_files_count; ++j) {
			if (temp_file_positions[j] == temp_file_lenghts[j]) {
				continue;
			}
			else if (min_element == nullptr) {
				min_element_block = j;
				min_element = &buffers[j][temp_file_positions[j] % block_lenght];
			}
			else if (compare((void*) &buffers[j][temp_file_positions[j] % block_lenght], min_element) < 0)
			{
				min_element = &buffers[j][temp_file_positions[j] % block_lenght];
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
		temp_file_positions[min_element_block] += element_count;
		if (temp_file_positions[min_element_block] < temp_file_lenghts[min_element_block] &&
			temp_file_positions[min_element_block] % block_lenght == 0) {
			fread(buffers[min_element_block], element_size / element_count, std::min(temp_file_lenghts[min_element_block] - temp_file_positions[min_element_block], block_lenght), temp_files[min_element_block]);
		}
	}

	// Flushing what's left
	if (write_buffer_position != 0) {
		fwrite(write_buffer, element_size / element_count, write_buffer_position, output);
		write_buffer_position = 0;
	}


	for (int i = 0; i < temp_files_count; ++i) {
		fclose(temp_files[i]);
	}

	fclose(output);

	for (int i = 0; i < temp_files_count; ++i) {
		delete[] buffers[i];
	}
	delete[] buffers;
	delete[] write_buffer;
	delete[] temp_file_lenghts;
	delete[] temp_file_positions;
	delete[] temp_files;
}

void create_test_input() {
	FILE *output = NULL;

	errno_t err = 0;
	err = fopen_s(&output, "input.bin", "wb");
	if (err != 0) {
		printf("Error opening test file\n");
		return;
	}

	int count = 1;
	fwrite(&count, sizeof(int), 1, output);

	UINT temp = 4;
	fwrite(&temp, sizeof(UINT), 1, output);
	temp = 5;
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

void join(const char* input1_filename, const char* input2_filename, const char* output_filename) {
	errno_t err = 0;

	FILE *input1 = NULL;
	err = fopen_s(&input1, input1_filename, "rb");
	if (err != 0) {
		printf("Error opening input1 file\n");
		return;
	}

	FILE *input2 = NULL;
	err = fopen_s(&input2, input2_filename, "rb");
	if (err != 0) {
		printf("Error opening input2 file\n");
		return;
	}

	FILE *output = NULL;
	err = fopen_s(&output, output_filename, "wb");
	if (err != 0) {
		printf("Error opening join file\n");
		return;
	}

	int count1;
	fread(&count1, sizeof(int), 1, input1);

	int count2;
	fread(&count2, sizeof(int), 1, input2);

	if (count1 != count2) {
		printf("Can't merge lists of different lenght\n");
		return;
	}

	const int available_bytes = 256 * 384;
	const int base_buffer_lenght = available_bytes / 7 / sizeof(UINT);

	UINT* input_buffer1 = new UINT[base_buffer_lenght * 2];
	UINT* input_buffer2 = new UINT[base_buffer_lenght * 2];
	UINT* output_buffer = new UINT[base_buffer_lenght * 3];

	int count_left = count1;
	const int io_number = count1 * sizeof(UINT) / base_buffer_lenght + ((count1 * sizeof(UINT) % base_buffer_lenght) ? 1 : 0);

	fwrite(&count_left, sizeof(int), 1, output);
	for (int i = 0; i < io_number; ++i) {
		fread(input_buffer1, sizeof(UINT), base_buffer_lenght * 2, input1);
		fread(input_buffer2, sizeof(UINT), base_buffer_lenght * 2, input2);

		for (int j = 0; j < std::min(base_buffer_lenght, count_left); ++j) {
			output_buffer[j * 3] = input_buffer2[j * 2];
			output_buffer[j * 3 + 1] = input_buffer2[j * 2 + 1];
			output_buffer[j * 3 + 2] = input_buffer1[j * 2 + 1];
		}

		fwrite(output_buffer, sizeof(UINT), std::min(count_left * 3, base_buffer_lenght * 3), output);
	}

	delete[] input_buffer1;
	delete[] input_buffer2;
	delete[] output_buffer;

	fclose(input1);
	fclose(input2);
	fclose(output);
}

int main()
{
	create_test_input();
	display_result("input.bin", 2, 0);

	const auto startTime = std::clock();

	ext_sort("input.bin", "temp1.bin", 2, 2 * sizeof(UINT), compare1, true);
	display_result("temp1.bin", 2, 0);

	ext_sort("temp1.bin", "temp2.bin", 2, 2 * sizeof(UINT), compare2, true);
	display_result("temp2.bin", 2, 0);

	join("temp1.bin", "temp2.bin", "join.bin");
	display_result("join.bin", 3, 0);

	ext_sort("join.bin", "output.bin", 3, 3 * sizeof(UINT), compare1, false);
	display_result("output.bin", 3, 1);

	// Calculating execution time
	const auto endTime = std::clock();
	double time = double(endTime - startTime) / CLOCKS_PER_SEC;
	std::cerr << "Time: " << time << '\n';

	return 0;
}
