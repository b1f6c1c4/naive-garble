#include <iostream>
#include <tomcrypt.h>

int main()
{
	register_all_ciphers();
	register_all_hashes();
	register_all_prngs();

	std::cout << "hello world" << std::endl;
	return 0;
}
