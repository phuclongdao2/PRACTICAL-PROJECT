#include <iostream>
#include <ctime>
#include <random>
using namespace std;

string help=
"----Rock Paper Scissors----\n"
"- Make one move type: r (rock), p (paper), s (scissors)\n"
"- Win: 2 points, Draw: 1 point, Lose: 0 points\n"
"- Reset score type: reset\n"
"- Quit game type: exit\n\n";

int main(){
	string inp; int player, computer, playerscore=0, computerscore=0; cout << help;
	while(1){
		cout << "Your move: "; getline(cin,inp);
		if (inp.compare("R")==0||inp.compare("r")==0) player=0;
		else if (inp.compare("P")==0||inp.compare("p")==0) player=1;
		else if (inp.compare("S")==0||inp.compare("s")==0) player=2;
		else if (inp.compare("exit")==0) return 0;
		else if (inp.compare("reset")==0){cout << "Score reset\n"; playerscore=0; computerscore=0; continue;}
		else{cout << "Invalid input\n"; continue;}
		mt19937 mt(time(nullptr)); computer=mt()%3;
		if (player==computer){
			cout << "Game Draw!\n"; playerscore++; computerscore++;
		}
		else if ((player==1&&computer==0)||(player==2&&computer==1)||(player==0&&computer==2)){
			cout << "Congratulations! You won this game!\n"; playerscore+=2;
		}
		else{
			cout << "Oh! Computer won this game!\n"; computerscore+=2;
		}
		if (player==0) cout << "Your move: " << "Rock";
		else if (player==1) cout << "Your move: " << "Paper";
		else if (player==2) cout << "Your move: " << "Scissors";
		if (computer==0) cout << " - Computer move: " << "Rock\n";
		else if (computer==1) cout << " - Computer move: " << "Paper\n";
		else if (computer==2) cout << " - Computer move: " << "Scissors\n";
		cout << "Player: " << playerscore << " - " << "Computer: " << computerscore << "\n\n";
	}
}
