#include <cmath>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <iomanip>

#include "Eigen/Dense"

using namespace std;
using namespace Eigen;

struct Vector2 {
	double x, y;
	Vector2(double x, double y) {
		this->x = x;
		this->y = y;
	}
};

struct Vector3 {
	double x, y, z;
	Vector3(double x, double y, double z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};


string inputFilename, outputFilename;

// === VARIABLE DECLARATIONS

//Vector3 offset(36, 35, 50);

Vector3 nozzleCompensation(0, 0, 0);
Vector3 currentPosition(0, 0, 0);

// change these
double S0 = 90;
double S1 = 0;
Vector2 nozzleOffset00(47, 26); // nozzle coordinates when S0 = 0 and S1 = 0
Vector3 nozzleOffset(72, -16, 85); // extents of the nozzle when swinging, XY coordinates when S0 = 90 and S1 = 0, and Z height when S1 = 90
double dist = 67; // distance of X from when S0 = 0 and S1 = 0 to when S0 = 0 and S1 = 90
Vector2 bedOffset(100, 100); // your origin is (0,0), the center of the bed is?
Vector3 partOffset(0, 0, 0); // where you want the center point of the base of the the part to be relative to the bed offset

// lists
vector<string> fileLinesArray;
vector<string> commandsArray;
vector<vector<double> > xyzArray;
vector<vector<string> > parametersArray;
vector<int> lineNumbersArray;

// matrix
Matrix<double, Dynamic, Dynamic, RowMajor> xyzMatrix;
MatrixXd rotationXMatrix(3,3);
MatrixXd rotationYMatrix(3,3);

// for settings
int choice;
int choice1;
void settings();
void mainSettings();
void otherSettings();
void menuOtherSettings();

int main() {
	// settings
    settings();
    cout << "Enter input filename: ";
    cin >> inputFilename;
    cout << "Enter output filename: ";
    cin >> outputFilename;

	// === MATH TIME
	// the values below represent where the nozzle is versus where it should be. subtracting the values above to the current position should compensate for where the nozzle is when rotating
	nozzleCompensation.x = nozzleOffset.x / 2 + cos((S0 - atan(nozzleOffset.x / nozzleOffset.y)*180.0f/M_PI)*M_PI/180.0f)*sqrt(pow(nozzleOffset.x / 2, 2) + pow(nozzleOffset.y / 2, 2)) + (sin((S0 + 90)*M_PI/180.0f)*dist*sin(S1*M_PI/180.0f));
	nozzleCompensation.y = nozzleOffset.y / 2 + sin((S0 - atan(nozzleOffset.x / nozzleOffset.y)*180.0f/M_PI)*M_PI/180.0f)*sqrt(pow(nozzleOffset00.x - nozzleOffset.x / 2.0f, 2) + pow(nozzleOffset00.y - nozzleOffset.y / 2.0f, 2)) + (cos((S0 + 90)*M_PI/180.0f)*dist*sin(S1*M_PI/180.0f));
	nozzleCompensation.z = cos((90 - abs(S1))*M_PI/180.0f) * nozzleOffset.z;
	//cout << "; Nozzle Compensation: (" << nozzleCompensation.x << "," << nozzleCompensation.y << "," << nozzleCompensation.z << ")\n";

	// === FILE READING
	string line;
	ifstream file(inputFilename);
	int currentLineNumber = 0;
	while (getline(file, line)) {
		istringstream iss(line);
		fileLinesArray.push_back(iss.str());
		string command, p0, p1, p2, p3, p4;
		iss >> command >> p0 >> p1 >> p2 >> p3 >> p4; // G0/G1 X Y Z E F should be the most input encountered
		//stringstream g01;
		if (command == "G0" || command == "G1") {
			//g01 << command;
			vector<string> extraParams;
			if (!p0.empty()) {
				if (p0.at(0) == 'X') {
					currentPosition.x = atof(p0.substr(1, string::npos).data());
					//g01 << " " << p0;
				}
				if (p0.at(0) == 'Y') {
					currentPosition.y = atof(p0.substr(1, string::npos).data());
					//g01 << " " << p0;
				}
				if (p0.at(0) == 'Z') {
					currentPosition.z = atof(p0.substr(1, string::npos).data());
					//g01 << " " << p0;
				}
				if (p0.at(0) == 'E' || p0.at(0) == 'F') extraParams.push_back(p0);
			}
			if (!p1.empty()) {
				if (p1.at(0) == 'X') {
					currentPosition.x = atof(p1.substr(1, string::npos).data());
					//g01 << " " << p1;
				}
				if (p1.at(0) == 'Y') {
					currentPosition.y = atof(p1.substr(1, string::npos).data());
					//g01 << " " << p1;
				}
				if (p1.at(0) == 'Z') {
					currentPosition.z = atof(p1.substr(1, string::npos).data());
					//g01 << " " << p1;
				}
				if (p1.at(0) == 'E' || p1.at(0) == 'F') extraParams.push_back(p1);//g01 << " " << p1;
			}
			if (!p2.empty()) {
				if (p2.at(0) == 'X') {
					currentPosition.x = atof(p2.substr(1, string::npos).data());
					//g01 << " " << p2;
				}
				if (p2.at(0) == 'Y') {
					currentPosition.y = atof(p2.substr(1, string::npos).data());
					//g01 << " " << p2;
				}
				if (p2.at(0) == 'Z') {
					currentPosition.z = atof(p2.substr(1, string::npos).data());
					//g01 << " " << p2;
				}
				if (p2.at(0) == 'E' || p2.at(0) == 'F') extraParams.push_back(p2);//g01 << " " << p2;
			}

			if (!p3.empty()) {
				if (p3.at(0) == 'E') extraParams.push_back(p3);//g01 << " " << p3;
			}
			if (!p4.empty()) {
				if (p4.at(0) == 'F') extraParams.push_back(p4); //g01 << " " << p4;
			}
			commandsArray.push_back(command);
            xyzArray.push_back({currentPosition.x, currentPosition.y, currentPosition.z});
			//xyzMatrix << currentPosition.x << currentPosition.y << currentPosition.z << endr;
			/*
			Row<double> newRow(3);
			newRow[0] = currentPosition.x;
			newRow[1] = currentPosition.y;
			newRow[2] = currentPosition.z;
			xyzMatrix.insert_rows(xyzMatrix.n_rows, newRow);*/
			parametersArray.push_back(extraParams);
			lineNumbersArray.push_back(currentLineNumber);
			//cout << "Current position: (" << currentPosition.x << "," << currentPosition.y << "," << currentPosition.z << ")\n";
			//fileLines.push_back(g01.str());
			//
		}

		currentLineNumber++;
	}

	// === MATRIX MATH TIME

	double rotX = S1 * cos((S0 + 90)*M_PI/180);
	double rotY = S1 * sin((S0 + 90)*M_PI/180);
	cout << "Rot X, Y: " << rotX << ", " << rotY << endl;
	cout << "Nozzle Compensation X, Y: " << nozzleCompensation.x << ", " << nozzleCompensation.y << endl;

	rotationXMatrix << 1, 0, 0,
                       0, cos(rotX*M_PI/180), sin(rotX*M_PI/180),
                       0, -sin(rotX*M_PI/180), cos(rotX*M_PI/180);

    rotationYMatrix << cos(rotY*M_PI/180), 0, sin(rotY*M_PI/180),
                        0, 1, 0,
                        -sin(rotY*M_PI/180), 0, cos(rotY*M_PI/180);

    cout << "Doing the matrix math.\n";
    int xyzSize = xyzArray.size();
    xyzMatrix.resize(xyzSize, 3);
    for(int i=0; i<xyzSize; i++) {
        xyzMatrix.row(i) = Vector3d(xyzArray[i][0], xyzArray[i][1], xyzArray[i][2]);
    }

    xyzMatrix = xyzMatrix * rotationXMatrix;
    xyzMatrix = xyzMatrix * rotationYMatrix;

    //cout << xyzMatrix << endl;

    xyzArray.clear();
    for(int i=0; i<xyzSize; i++) {
        xyzArray.push_back({xyzMatrix(i,0), xyzMatrix(i,1), xyzMatrix(i,2)});
    }
/*
    for (int i = 0; i < xyzMatrix.n_rows; i++) {
        xyzArray.push_back({*(xyzMatrix.colptr(0) + 0), *(xyzMatrix.colptr(0) + 0), *(xyzMatrix.colptr(0) + 0)});

    }*/



	// === MODIFYING OF LINES FOR NEW FILE
	int sizeI = lineNumbersArray.size();
	for (int i = 0; i < sizeI; i++) {
		string extraParams = "";
		stringstream lineToWrite;
		for (int j = 0; j < parametersArray[i].size(); j++) {
			extraParams += " " + parametersArray[i][j];
		}

		// apply all the offsets, round to 6th decimal place
		// rounding from: http://www.cplusplus.com/forum/beginner/3600/
		long double finalX = xyzArray[i][0] - nozzleCompensation.x + bedOffset.x + partOffset.x;
		long double finalY = xyzArray[i][1] - nozzleCompensation.y + bedOffset.y + partOffset.y;
		long double finalZ = xyzArray[i][2] - nozzleCompensation.z + partOffset.z;

		finalX = ceil((finalX*pow(10,10)) - 0.49) / pow(10,10);
		finalY = ceil((finalY*pow(10,10)) - 0.49) / pow(10,10);
		finalZ = ceil((finalZ*pow(10,10)) - 0.49) / pow(10,10);

        lineToWrite << commandsArray[i] << " X" << finalX << " Y" << finalY << " Z" << finalZ << extraParams;
		fileLinesArray[lineNumbersArray[i]] = lineToWrite.str();
	}

	// === WRITE NEW FILE
	// currently just outputting to cli
	ofstream newFile;
	newFile.open(outputFilename);
	newFile << "M280 P0 S" << S0 << endl;
	newFile << "G4 S100" << endl;
	newFile << "M280 P1 S" << S1 << endl;
	newFile << "G4 S100" << endl;
	for (int i = 0; i < fileLinesArray.size(); i++) {
		newFile << fileLinesArray[i] << endl;
	}

	newFile.close();

	return 0;
}

void settings(){
    do{
        mainSettings();
        switch(choice){
            case 1:
                cout << "CHANGE SERVO ROTATIONS AND PART OFFSET:" << endl;
                cout << "Change Servo Rotation:" << endl;
                cout << "S0 = ";
                cin >> S0;
                cout << "S1 = ";
                cin >> S1;
                cout << endl;
                cout << "Change Part Offset:" << endl;
                cout << "x = ";
                cin >> partOffset.x;
                cout << "y = ";
                cin >> partOffset.y;
                cout << "z = ";
                cin >> partOffset.z;
                cout << endl;
                break;
            case 2:
                otherSettings();
                break;
            case 3:
                cout << endl;
                break;
            default:
                cout << "ERROR. Not an option." << endl;
                break;
        }
    }while (choice !=3);
}

void otherSettings(void){
    do{
        menuOtherSettings();
        switch(choice1){
            case 1:
                cout << "CHANGE NOZZLE OFFSETS:" << endl;
                cout << "Nozzle Offset when S0 =0 and S1 = 0:" << endl;
                cout << "x = ";
                cin >> nozzleOffset00.x;
                cout << "y = ";
                cin >> nozzleOffset00.y;
                cout << endl;

                cout << "Nozzle Offset:" << endl;
                cout << "x = ";
                cin >> nozzleOffset.x;
                cout << "y = ";
                cin >> nozzleOffset.y;
                cout << "z = ";
                cin >> nozzleOffset.z;
                cout << endl;
                break;
            case 2:
                cout << "CHANGE BED OFFSET:" << endl;
                cout << "Bed Offset:" << endl;
                cout << "x = ";
                cin >> bedOffset.x;
                cout << "y = ";
                cin >> bedOffset.y;
                cout << endl;
                break;
            case 3:
                cout << "CHANGE DISTANCE OF NOZZLES:" << endl;
                cout << "Distance = ";
                cin >> dist;
                cout << endl;
                break;
            case 4:
                settings();
                break;
            default:
                cout << "ERROR. Not an option." << endl;
                break;
        }
    }while (choice1 !=4);
}

void mainSettings(void){
    cout << "==================================================" << endl;
    cout << "CURRENT SETTINGS:" << endl;
    cout << "S0 = " << S0 << endl;
    cout << "S1 = " << S1 << endl;
    cout << "Part Offset (" << partOffset.x << "," << partOffset.y << "," << partOffset.z << ")" << endl;
    cout << endl << endl;

    cout << "CHANGE SETTINGS:" << endl;
    cout << "[1] Change Servo Rotations and Part Offset" << endl;
    cout << "[2] Other Settings" << endl;
    cout << "[3] No Changes" << endl;
    cout << endl;
    cout << "Choice: ";
    cin >> choice;
    cout << endl;
}

void menuOtherSettings(void){
    cout << "==================================================" << endl;
    cout << "OTHER SETTINGS:" << endl;
    cout << "Nozzle Offset when S0 =0 and S1 = 0: (" << nozzleOffset00.x << "," << nozzleOffset00.y << ")" << endl;
    cout << "Nozzle Offset: (" << nozzleOffset.x << "," << nozzleOffset.y << "," << nozzleOffset.z << ")" << endl;
    cout << "Bed Offset: (" << bedOffset.x << "," << bedOffset.y <<  ")" << endl;
    cout << "Distance =  " << dist << endl;
    cout << endl;
    cout << "CHANGE SETTINGS:" << endl;
    cout << "[1] Change Nozzle Offsets" << endl;
    cout << "[2] Change Bed Offset" << endl;
    cout << "[3] Change Distance" << endl;
    cout << "[4] No Changes" << endl;
    cout << endl;
    cout << "Choice: ";
    cin >> choice1;
    cout << endl;
}
