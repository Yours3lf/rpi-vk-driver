#include <iostream>
#include <vector>
#include <algorithm>
#include <string.h>

#include "../inputHandler/inputHandler.h"

int main(void) {
		initInputHandler();

		while(true)
		{
			handleInput();
		}

		uninitInputHandler();

		return 0;
}
