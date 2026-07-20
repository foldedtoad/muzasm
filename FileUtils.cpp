//
//  FileUtils.cpp
//  MUZ-Workshop
//
//  Created by Francis Pierot on 28/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//
//#include "pch.h"
#include "FileUtils.h"
#include <stdio.h>
#include <sys/stat.h>

bool ExistFile(std::string file)
{
	FILE* f = fopen(file.c_str(), "r");
	if (f) {
		fclose(f);
		return true;
	}
	return false;
}
bool ExistDir(std::string dir)
{
	struct stat st;
	int result = stat(dir.c_str(), &st);
	if (result == 0 && (st.st_mode & S_IFDIR)){
		return true;
	}
	return false;
}

