#include <cmath>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "armadillo"

//#define M_PI 3.14159265358979323846
long double M_PI = atan(1.0f)*4;

using namespace std;
using namespace arma;

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

const char* filename = "test.gcode";

int main() {
	// === VARIABLE DECLARATIONS

	//Vector3 offset(36, 35, 50);

	Vector3 nozzleCompensation(0, 0, 0);
	Vector3 currentPosition(0, 0, 0);

	// change these
	double S0 = -90;
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
	mat xyzMatrix;
	mat rotationXMatrix;
	mat rotationYMatrix;

	// settings
    int choice;
    int choice1;
    cout << "CURRENT SETTINGS:" << endl;
    cout << "S0 = " << S0 << endl;
    cout << "S1 = " << S1 << endl;
    cout << "Part Offset (" << partOffset.x << "," << partOffset.y << "," << partOffset.z << ")" << endl;
    cout << endl << endl;

    cout << "CHANGE SETTINGS:" << endl;
    cout << "[1] Change Servo Rotations and Part Offset" << endl;
    cout << "[2] Other Settings" << endl;
    cout << "[3] No Changes" << endl;
    cin >> choice;

    switch(choice){
        case 1:
            cout << endl;
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
            cout << endl << endl;
            break;
        case 2:
            cout << endl;
            cout << "CURRENT SETTINGS:" << endl;
            cout << "Nozzle Offset when S0 =0 and S1 = 0: (" << nozzleOffset00.x << "," << nozzleOffset00.y << ")" << endl;
            cout << "Nozzle Offset: (" << nozzleOffset.x << "," << nozzleOffset.y << "," << nozzleOffset.z << ")" << endl;
            cout << "Bed Offset: (" << bedOffset.x << "," << bedOffset.y <<  ")" << endl;
            cout << "Distance =  " << dist << endl;
            cout << endl;
            cout << "CHANGE SETTINGS:"<< endl;
            cout << "[1] Change Nozzle Offsets" << endl;
            cout << "[2] Change Bed Offset" << endl;
            cout << "[3] Change Distance" << endl;
            cout << "[4] No Changes"<< endl;
            cin >> choice1;
                switch(choice1){
                case 1:
                    cout << endl;
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
                    cout << endl << endl;
                    break;
                case 2:
                    cout << endl;
                    cout << "CHANGE BED OFFSET:" << endl;
                    cout << "Bed Offset:" << endl;
                    cout << "x = ";
                    cin >> bedOffset.x;
                    cout << "y = ";
                    cin >> bedOffset.y;
                    cout << endl << endl;
                    break;
                case 3:
                    cout << endl;
                    cout << "CHANGE DISTANCE OF NOZZLES:" << endl;
                    cout << "Distance = ";
                    cin >> dist;
                    cout << endl << endl;
                break;
                case 4:
                    cout << endl << endl;
                    break;
                default:
                    cout << "ERROR. Not an option." << endl << endl;
                    break;

                }

            break;
        case 3:
            cout << endl << endl;
            break;
        default:
            cout << "ERROR. Not an option." << endl << endl;
            break;

    }


	// === MATH TIME
	// the values below represent where the nozzle is versus where it should be. subtracting the values above to the current position should compensate for where the nozzle is when rotating
	nozzleCompensation.x = nozzleOffset.x / 2 + cos((S0 - atan(nozzleOffset.x / nozzleOffset.y)*180.0f/M_PI)*M_PI/180.0f)*sqrt(pow(nozzleOffset.x / 2, 2) + pow(nozzleOffset.y / 2, 2)) + (sin((S0 + 90)*M_PI/180.0f)*dist*sin(S1*M_PI/180.0f));
	nozzleCompensation.y = nozzleOffset.y / 2 + sin((S0 - atan(nozzleOffset.x / nozzleOffset.y)*180.0f/M_PI)*M_PI/180.0f)*sqrt(pow(nozzleOffset00.x - nozzleOffset.x / 2.0f, 2) + pow(nozzleOffset00.y - nozzleOffset.y / 2.0f, 2)) + (sin((S0 + 90)*M_PI/180.0f)*dist*sin(S1*M_PI/180.0f));
	nozzleCompensation.z = cos((90 - abs(S1))*M_PI/180.0f) * nozzleOffset.z;
	cout << "; Nozzle Compensation: (" << nozzleCompensation.x << "," << nozzleCompensation.y << "," << nozzleCompensation.z << ")\n";

	// === FILE READING
	string line;
	ifstream file(filename);
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
					currentPosition.x = atof(p0.substr(1, string::npos).data()) - nozzleCompensation.x + bedOffset.x;
					//g01 << " " << p0;
				}
				if (p0.at(0) == 'Y') {
					currentPosition.y = atof(p0.substr(1, string::npos).data()) - nozzleCompensation.y + bedOffset.y;
					//g01 << " " << p0;
				}
				if (p0.at(0) == 'Z') {
					currentPosition.z = atof(p0.substr(1, string::npos).data()) - nozzleCompensation.z;
					//g01 << " " << p0;
				}
				if (p0.at(0) == 'E' || p0.at(0) == 'F') extraParams.push_back(p0);
			}
			if (!p1.empty()) {
				if (p1.at(0) == 'X') {
					currentPosition.x = atof(p1.substr(1, string::npos).data()) - nozzleCompensation.x + bedOffset.x;
					//g01 << " " << p1;
				}
				if (p1.at(0) == 'Y') {
					currentPosition.y = atof(p1.substr(1, string::npos).data()) - nozzleCompensation.y + bedOffset.y;
					//g01 << " " << p1;
				}
				if (p1.at(0) == 'Z') {
					currentPosition.z = atof(p1.substr(1, string::npos).data()) - nozzleCompensation.z;
					//g01 << " " << p1;
				}
				if (p1.at(0) == 'E' || p1.at(0) == 'F') extraParams.push_back(p1);//g01 << " " << p1;
			}
			if (!p2.empty()) {
				if (p2.at(0) == 'X') {
					currentPosition.x = atof(p2.substr(1, string::npos).data()) - nozzleCompensation.x + bedOffset.x;
					//g01 << " " << p2;
				}
				if (p2.at(0) == 'Y') {
					currentPosition.y = atof(p2.substr(1, string::npos).data()) - nozzleCompensation.y + bedOffset.y;
					//g01 << " " << p2;
				}
				if (p2.at(0) == 'Z') {
					currentPosition.z = atof(p2.substr(1, string::npos).data()) - nozzleCompensation.z;
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
			xyzMatrix << currentPosition.x << currentPosition.y << currentPosition.z << endr;
			xyzArray.push_back({ currentPosition.x, currentPosition.y, currentPosition.z });
			parametersArray.push_back(extraParams);
			lineNumbersArray.push_back(currentLineNumber);
			//cout << "Current position: (" << currentPosition.x << "," << currentPosition.y << "," << currentPosition.z << ")\n";
			//fileLines.push_back(g01.str());
		}

		currentLineNumber++;
	}

	// === MATRIX MATH TIME

	// === MODIFYING OF LINES FOR NEW FILE
	int sizeI = lineNumbersArray.size();
	for (int i = 0; i < sizeI; i++) {
		string extraParams = "";
		stringstream lineToWrite;
		for (int j = 0; j < parametersArray[i].size(); j++) {
			extraParams += " " + parametersArray[i][j];
		}
		 lineToWrite << commandsArray[i] << " X" << xyzArray[i][0] << " Y" << xyzArray[i][1] << " Z" << xyzArray[i][2] << extraParams;
		 fileLinesArray[lineNumbersArray[i]] = lineToWrite.str();
	}

	// === WRITE NEW FILE
	// currently just outputting to cli
	cout << "M280 P0 S" << S0 << endl;
	cout << "G4 S100" << endl;
	cout << "M280 P1 S" << S1 << endl;
	cout << "G4 S100" << endl;
	for (int i = 0; i < fileLinesArray.size(); i++) {
		cout << fileLinesArray[i] << endl;
	}

	return 0;
}
