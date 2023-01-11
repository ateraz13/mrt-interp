#include <iostream>
#include <cstdint>
#include <cstring>
#include <cmath>

int32_t str_to_i32(const char* str) 
{
	bool negative = false;
	if(*str == '-') {
		negative = true;
		str++;
	}			

	//std::cout << "FIRST CHAR: " << *str << std::endl;

	int32_t num = 0;
	size_t  len = strlen(str);
	size_t	idx = 0;

	while(*str != '\0') {
		num += (*str - '0') * pow(10, (len-idx)-1);	
		//std::cout << "dig: " << *str << ", len-idx: " << len-idx << ", pow(10, (len-idx)-1): " << pow(10, (len-idx)-1) << std::endl;

		idx++;
		str++;
	}	

	return negative ? -num : num;
}

uint32_t str_to_u32(const char* str) 
{
	uint32_t num = 0;
	size_t  len = strlen(str);
	size_t	idx = 0;

	while(*str != '\0') {
		num += (*str - '0') * pow(10, (len-idx)-1);	
		//std::cout << "dig: " << *str << ", len-idx: " << len-idx << ", pow(10, (len-idx)-1): " << pow(10, (len-idx)-1) << std::endl;

		idx++;
		str++;
	}	

	return num;
}


void print_usage(int argc, char** argv) {
	std::cout << argv[0] << ": <signed/unsigned>  [[DECIMAL NUMBERS].../[<range> <MIN> <MAX>" << std::endl;
}

int main(int argc, char** argv)
{
	if(argc < 3) {
		print_usage(argc, argv);
		return 0;
	}

	bool use_signed = false;

	if(strncmp("unsigned", argv[1], 8) == 0) {
		use_signed = false;
	}
	else if(strncmp("signed", argv[1], 6) == 0) {
		use_signed = true;
	}
	else {
		std::cout << "Expected \"signed\" or \"unsigned\" argument." << std::endl;
		return 1;
	}	

	uint8_t bytes[4] = {0};
	bool ranged = false;
	
	if(strncmp(argv[2], "ranged", 6) == 0) {
		printf("Ranged sequence from %s to %s", argv[4], argv[5]);
		ranged = true;
	}

	if(!ranged) {
		std::cout << "Processing number..." << std::endl;
		if(use_signed) {
			for(int i = 2; i < argc; i++) {
				std::cout << "Using signed integers\n";
				int32_t num = str_to_i32(argv[i]);
				memcpy(bytes, &num, 4);
				std::cout << "signed int: "  << num << " = 0x" << std::hex << (int)bytes[0] << " 0x" << (int)bytes[1] << " 0x" << (int)bytes[2] << " 0x" << (int)bytes[3] << std::endl;
			}
		}
		else {
			for(int i = 2; i < argc; i++) {
				//std::cout << "Using unsigned integers\n";
				uint32_t num = str_to_u32(argv[i]);
				memcpy(bytes, &num, 4);
				std::cout << "unsigned int: "  << num << " = 0x" << std::hex << (int)bytes[0] << " 0x" << (int)bytes[1] << " 0x" << (int)bytes[2] << " 0x" << (int)bytes[3] << std::endl;
			}
		}
	}
	else {
		if(argc < 5) {
			print_usage(argc, argv);
		}

		int min = str_to_i32(argv[3]);
		int max = str_to_i32(argv[4]);

		if(use_signed) {
			for(int i = min; i < max; i++) {
				//std::cout << "Using signed integers\n";
				int32_t num = i;
				memcpy(bytes, &num, 4);
				std::cout << "signed int: "  << std::dec << num << " = 0x" << std::hex << (int)bytes[0] << " 0x" << (int)bytes[1] << " 0x" << (int)bytes[2] << " 0x" << (int)bytes[3] << std::endl;
			}
		}
		else {
			for(int i = min; i < max; i++) {
				//std::cout << "Using unsigned integers\n";
				uint32_t num = i;
				memcpy(bytes, &num, 4);
				std::cout << "unsigned int: "  << num << " = 0x" << std::hex << (int)bytes[0] << " 0x" << (int)bytes[1] << " 0x" << (int)bytes[2] << " 0x" << (int)bytes[3] << std::endl;
			}
		}
	
	}
	return 0;
}
