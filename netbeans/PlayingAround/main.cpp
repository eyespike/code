/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Jeff
 *
 * Created on December 1, 2016, 3:17 PM
 */

#include <iostream>

using namespace std;

/*
 * 
 */
int getValueFromUser(){
	
	std::cout << "Enter an integer: ";
	int i;
	std::cin >> i;
	return i;	
}



int main() {
	
	int x = getValueFromUser();
	int y = getValueFromUser();
	
	std::cout << x << " - " << y << " = " << x - y << std::endl;
	
	return 0;	
}

