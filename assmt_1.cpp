#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <bitset>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <cmath>

using namespace std;

int sp_text_seg_address = 0x00400000;
int sp_data_seg_address = 0x10000000;
int data_sect = 0;
int text_sect = 0;

set<string>inst_set = {"addu", "and", "jr", "nor", "or", "sltu", "sll", "srl", "subu", "addiu", "andi", "beq", "bne", "lui", "lw", "ori", "sltiu", "sw", "lb", "sb", "j", "jal", "la"};


set<string>R_set = {"addu", "and", "nor", "or", "sltu", "subu"};
set<string>R_set1 = {"jr"};
set<string>R_set2 = {"sll", "srl"};


set<string>I_set = {"sltiu", "ori", "lui", "andi", "addiu"};
set<string>I_set1 = {"lw", "lb", "sw", "sb"};
set<string>I_set2 = {"bne", "beq"};

map<string, int> R_fm = {{"addu", 0x21}, {"and", 0x24}, {"jr", 8}, {"nor", 0x27}, {"or", 0x25}, {"sltu", 0x2b}, {"sll", 0}, {"srl", 2}, {"subu", 0x23}};



map<string, int> I_fm = {{"addiu", 9},{"andi", 0xc}, {"beq", 4}, {"bne", 5}, {"lui", 0xf}, {"lw", 0x23}, {"ori", 0xd}, {"sltiu", 0xb}, {"sw", 0x2b}, {"lb", 0x20}, {"sb", 0x28}};



map<string, int> J_fm = {{"j", 2}, {"jal", 3}};

map<string, int>label;
map<string, int>label_data;
vector<string>data_vec;
vector<string>exe;
vector<string>sol_f;

void data_slicing(string inputf);
void text_slicing(string inputf);
int tobinary(vector<string>real_exe, int pc_);

int main(int argc, char* argv[]){

	string input_file = argv[1]; //input filename of .s file
	data_slicing(input_file);
	text_slicing(input_file);
	int pc = sp_text_seg_address;
	vector<string>real_exe;
	for(auto i:exe){
		if(inst_set.find(i) != inst_set.end()){
			if(tobinary(real_exe, pc) != 0){
				pc=pc+4;
				
			}		
			real_exe.clear();
			real_exe.push_back(i);
		}
		else
			real_exe.push_back(i);
	}
	tobinary(real_exe, pc);
	int sabin = 0;	
	for(auto i:sol_f){
		sabin++;
	}
	string output_file = input_file.substr(0,input_file.find(".")+1)+"o";
	ofstream of;
	of.open(output_file);
	ostringstream ss;
	ss << "0x" << hex << sol_f.size()*4;
	string result = ss.str();
	of << result + "\n";
	ostringstream ss1;
	ss1 << "0x" << hex << data_sect*4;
	result = ss1.str();
	of << result + "\n";
	for(auto i:sol_f)
		of << i + "\n";
	for(auto k:data_vec){
		if(k.find("0x") != string::npos)
			of << k + "\n";
		else
			of << "0x" + k + "\n";
	}
	of.close();
}


void data_slicing(string inputf){

	ifstream file;
	file.open(inputf);
	string line;
        string data;
        string data2;
        while(getline(file, line)){
                if(line.find(":") != string::npos){
                        data = line.substr(0, line.find(":"));
                        data2 = line.substr(line.find(":")+1);
                        label_data.insert({data,sp_data_seg_address + data_sect*4});
                        if(data2 != ""){
                                data2 = line.substr(line.find(".word")+5);
                                data2.erase(remove(data2.begin(), data2.end(), ' '), data2.end());
				if(data2.find("0x") != string::npos){
					for(int i =0; i != data2.size() ; i++){
						data2[i] = tolower(data2[i]);
					}
				}
				else{
					ostringstream ss;
    					ss << hex << stoi(data2);
    					data2 = ss.str();
				}
                                data_vec.push_back(data2);
                                data_sect++;
                        }

                }
                else if (line != "" and line.find(".word") != string::npos) {
                        data2 = line.substr(line.find(".word")+5);
                        data2.erase(remove(data2.begin(), data2.end(), ' '), data2.end());
                        if(data2.find("0x") != string::npos){
                                        for(int i =0; i != data2.size() ; i++){
                                                data2[i] = tolower(data2[i]);
                                        }
                                }
                        else{
                                        ostringstream ss;
                                        ss << hex << stoi(data2);
                                        data2 = ss.str();
                        }
			data_vec.push_back(data2);
                        data_sect++;
                }
                if(line.find(".text") != string::npos){
                        break;
                }

        }
	file.close();

	
}
void text_slicing(string inputf){
	
	ifstream file;
	file.open(inputf);
	string line;
        string sub;
        string sub2;
	char * inst;
        while(getline(file, line)){
                if(line.find(".text") != string::npos ) 
                        break;
        }
        while(getline(file, line)){
                if(line.find(":") != string::npos){
                        sub = line.substr(0,line.find(":"));
                        sub2 = line.substr(line.find(":")+1);
                        label.insert({sub,sp_text_seg_address + text_sect*4});
                        if(sub2 != ""){
				inst = strcpy(new char[sub2.length() + 1], sub2.c_str());
			  	char * ptr = strtok(inst, ", ");
				while(ptr != NULL){
					
					exe.push_back(string(ptr));
					ptr = strtok(NULL, ", ");

				}
                                text_sect++;
                        }
                        else
                                continue;
                }
                else if (line != ""){
                        inst = strcpy(new char[line.length() + 1], line.c_str());
			char * ptr = strtok(inst, ", ");
			while(ptr != NULL){
			exe.push_back(string(ptr));
                        	ptr = strtok(NULL, ", ");
                        }

                        text_sect++;
                }

        }
	file.close();
}

int tobinary(vector<string>real_exe, int pc_){
	if(real_exe.size() == 0)
		return 0;
	bitset<32> sol;
	bitset<32> sol1;
	bitset<32> sol2;
	bitset<32> sol3;
	bitset<32> sol4;
	string func;
	func = real_exe[0];
	if(func == "la"){
		string rd = real_exe[1];
		string add = real_exe[2];
		rd = rd.substr(rd.find("$")+1);
		int add_;
		if(label_data.find(add) != label_data.end()){
			add_ = label_data[add];

		}
		else{
			if(add.find("0x") != string::npos){
                                char cast[40];
                                strcpy(cast, add.c_str());
                                add_ = (int)strtol(cast, NULL, 16);
                        }

                        else
                                add_ = stoi(add);
		}
		
		if((add_ & 65535) != 0){
			sol = I_fm["lui"];
			sol1 = stoi(rd);
			sol2 = stoi(rd);
			sol3 = add_;
			sol = sol << 26 | sol2 << 16 | sol3 >>16;
			ostringstream ss1;
                        ss1 << "0x" << hex << sol.to_ulong();
                        string result = ss1.str();
			sol_f.push_back(result);
			sol = I_fm["ori"];
			sol3 = add_&65535;
			sol = sol << 26 | sol1 << 21 | sol2 << 16 | sol3;
			
			ostringstream ss;
    			ss << "0x" << hex << sol.to_ulong();
    			result = ss.str();
			sol_f.push_back(result);

		}

		else{
			sol = I_fm["lui"];
			sol1 = stoi(rd);
			sol2 = add_;
			sol = sol << 26 | sol1 << 16 | sol2 >> 16;
			ostringstream ss;
                        ss << "0x" << hex << sol.to_ulong();
                        string result = ss.str();
			sol_f.push_back(result);


		}
	}

	else{
		if(R_fm.find(func) != R_fm.end()){
			
			sol4 = R_fm[func];
			sol = sol | sol4;
			if(R_set.find(func) != R_set.end()){
				string rd = real_exe[1];
				string rs = real_exe[2];
				string rt = real_exe[3];
				rd = rd.substr(rd.find("$")+1);
				rs = rs.substr(rs.find("$")+1);
				rt = rt.substr(rt.find("$")+1);
				sol1 = stoi(rd);
				sol2 = stoi(rs);
				sol3 = stoi(rt);
				sol = sol | sol1 << 11 | sol2 << 21 | sol3 << 16 ;
			}
			else if(R_set1.find(func) != R_set1.end()){
				
				string rs = real_exe[1];
				rs = rs.substr(rs.find("$")+1);
				sol1 = stoi(rs);
				sol = sol | sol1 << 21;
	
			}
			
	
			else{
				string rd = real_exe[1];
				string rt = real_exe[2];
				string shamt = real_exe[3];
				rd = rd.substr(rd.find("$")+1);
				rt = rt.substr(rt.find("$")+1);
				int shamt_;
				if(shamt.find("0x") != string::npos){
					char cast[10];
					strcpy(cast, shamt.c_str());
					shamt_ = (int)strtol(cast, NULL, 16);
				}
					
				else
					shamt_ = stoi(shamt);	
				sol1 = stoi(rd);
				sol2 = stoi(rt);
				sol3 = shamt_;
				sol = sol | sol1 << 11 | sol2 << 16 | sol3 << 6;
			}
		}
		
		else if(I_fm.find(func) != I_fm.end()){
			
			sol = I_fm[func];
			sol = sol << 26;
			if(I_set.find(func) != I_set.end()){
				if (func!="lui"){
					string rt = real_exe[1];
					string rs = real_exe[2];
					string imm = real_exe[3];
					int imm_;
					if(imm.find("0x") != string::npos){
						char cast[10];
	                                	strcpy(cast, imm.c_str());
	                                	imm_ = (int)strtol(cast, NULL, 16);
					}
					
					else
						imm_ = stoi(imm);
					sol3 = imm_ & 65535;
					rt = rt.substr(rt.find("$")+1);
					sol1 = stoi(rt);
					
					
					rs = rs.substr(rs.find("$")+1);
	                                sol2 = stoi(rs);
					sol = sol | sol1 << 16 | sol2 << 21 | sol3;
				
					
	
				}
	
	
	
				else{
					string rt = real_exe[1];
					string imm = real_exe[2];
					int sol3_int = stoi(imm);
					sol3 = sol3_int & 65535;
	
					rt = rt.substr(rt.find("$")+1);
	                                sol2 = stoi(rt);
	                                
					sol = sol | sol2 << 16 | sol3;
	
				}
	                }
			else if (I_set1.find(func) != I_set1.end()){
				string rt = real_exe[1]; 
				string rs = real_exe[2];
				int offset;
	                     
				rt = rt.substr(rt.find("$")+1);
	                        sol1 = stoi(rt);
	                     
	                        offset = stoi(rs.substr(0, rs.find("(")));
	                        rs = rs.substr(rs.find("$")+1, rs.find(")")-3);
	                        sol2 = stoi(rs);
				sol3 = offset;
	                        sol = sol | sol1 << 16 | sol2 << 21 | sol3;
			
	
	                }
			else{
				string rs = real_exe[1];
				string rt = real_exe[2];
				string lb = real_exe[3];
		       		rs = rs.substr(rs.find("$")+1);
	                        sol1 = stoi(rs);
	                        rt = rt.substr(rt.find("$")+1);
	                        sol2 = stoi(rt);
				sol3 = (label[lb]-4-pc_)/4;
	
				sol = sol | sol1 << 21 | sol2 << 16 | sol3;

				               
				
			}
		}
	
		else if(J_fm.find(func) != J_fm.end()){
			
			sol = J_fm[func];
			sol = sol << 26;
			string a = real_exe[1];
			sol1 = label[a]/4;
	
			sol = sol | sol1;
	
	
	
		}
		ostringstream ss;
                ss << "0x" << hex << sol.to_ulong();
                string result = ss.str();
		sol_f.push_back(result);
	
	}	
		return 1;	
}
