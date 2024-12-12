#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>

using namespace std;

// putting constants & global vars here
int WINDOW_W = 1280;
int WINDOW_H = 720;
int GAMESTATE = 0;  // 0 = menu/select, 1 = at least one game active, 2 = game over
bool FLAP_isActive = false;
bool SNAKE_isActive = false;
int SNAKE_FRAMETIMER = 0;
int SNAKE_FRAMETIMER_FRUIT = 0;

// GAMESTATE 0 DATA

// GAMESTATE 1 DATA
// bools for key state info
bool KEY_FLAP = false;
bool KEY_UP = false;
bool KEY_DOWN = false;
bool KEY_LEFT = false;
bool KEY_RIGHT = false;
bool KEY_ESCAPE = false;

// snake data
int SNAKE_SCORE = 0;
const int SNAKE_MAX_SCORE = 255; // 16*16, 256 blocks total. snake game resets after this
int SNAKE_DIR = -1;
int SNAKE_DX = -1;
int SNAKE_DY = -1;
int SNAKE_SPEED_MULT = 1;
int SNAKE_FRUIT_TEMPX = 0;
int SNAKE_FRUIT_TEMPY = 0;
const int SNAKE_BLOCK_UNIT = 40; // 640/16, 256 blocks total.
struct Block {
	int x = 0;
	int y = 0;
	sf::RectangleShape sf_block;

	Block(int x_in, int y_in){ // user must always specify an x and y to spawn instance.
		sf_block.setSize(sf::Vector2f(SNAKE_BLOCK_UNIT, SNAKE_BLOCK_UNIT));
		sf_block.setPosition(sf::Vector2f(x_in, y_in));
		sf_block.setFillColor(sf::Color::White);
	}
	void moveBlock(int dx, int dy) {
		int dx_adj = dx * SNAKE_BLOCK_UNIT;
		int dy_adj = dy * SNAKE_BLOCK_UNIT; //this below should actually be the game over condition
		if(!((sf_block.getPosition().x + dx_adj) < 640 || (sf_block.getPosition().x + dx_adj) > (1280-40))){
			sf_block.move(dx_adj, 0);
		}
		if(!((sf_block.getPosition().y + dy_adj) < 0 || (sf_block.getPosition().y + dy_adj) > (640-40))){
			sf_block.move(0, dy_adj);
		}
	}

};

// flappy bird data
int FLAP_SCORE = 0;
const float FLAP_UP_VEL = 16.0f; // THIS is a velocity to set to.
const float FLAP_DOWN_VEL = 0.98f; // this is acceleration, not velocity, oops too late to change name
const float FLAP_PIPE_SPEED = 2.75f * -1;
const int FLAP_PIPE_DIST = 150;
float FLAP_BIRD_YVEL = 0;
struct Pipe {
	sf::RectangleShape piperect;
	sf::RectangleShape gaprect;
	int gap;		

	Pipe(){
		piperect.setFillColor(sf::Color(45,175,25));
		gaprect.setFillColor(sf::Color(165,255,250));
		gap = static_cast<int>(rand() % (420)) + 64;	
		piperect.setPosition(640,0);
		gaprect.setPosition(640, gap - 128);
		piperect.setSize(sf::Vector2f(72, 640));
		gaprect.setSize(sf::Vector2f(72, 256));
	}
};

void writeLeaderboard(const string& leader_string) {
	ofstream file("Leaderboard.txt", ios::app);

	if(!file.is_open()) return;	
	file << leader_string << "\n";
	file.close();
	cout << "leaderboard written to";
}


int main() {	

	// goofy game title RNG
	srand(time(NULL));
	string titles[4] = {"FLAPPY SNAKE", "SNAKEY BIRD", "BIRDY SNAKE", "SNAKEY FLAP"};
	const string GAME_TITLE = titles[rand() % 4];

	// initialize window
	sf::RenderWindow window(sf::VideoMode(1280, 720), GAME_TITLE);
	window.setFramerateLimit(60);	

	// declare objects for drawing and game logic
	//text objs
	sf::Font normalfont;
	if(!normalfont.loadFromFile("arial.ttf")) return -1;
	sf::Text flappyScore_text;
	sf::Text snakeScore_text;
	sf::Text title_text;
	sf::Text tutorial_text;
	snakeScore_text.setFont(normalfont);
	flappyScore_text.setFont(normalfont);
	title_text.setFont(normalfont);
	tutorial_text.setFont(normalfont);
	snakeScore_text.setCharacterSize(32);
	flappyScore_text.setCharacterSize(32);
	title_text.setCharacterSize(72);
	tutorial_text.setCharacterSize(20);
	snakeScore_text.setFillColor(sf::Color::Black);
	flappyScore_text.setFillColor(sf::Color::Black);
	title_text.setFillColor(sf::Color::White);		
	tutorial_text.setFillColor(sf::Color::White);

	snakeScore_text.setPosition(sf::Vector2f(640,645));
	flappyScore_text.setPosition(sf::Vector2f(0,645));
	
	title_text.setString(GAME_TITLE);
	tutorial_text.setString("Press space to start!");
	title_text.setPosition(sf::Vector2f(20,20));
	tutorial_text.setPosition(sf::Vector2f(20,300));

	sf::RectangleShape rectangle_bg1(sf::Vector2f(640,640));
	sf::RectangleShape rectangle_bg2(sf::Vector2f(640,640));
	sf::RectangleShape rectangle_bg3(sf::Vector2f(1280,720));
	sf::RectangleShape rectangle_menubar(sf::Vector2f(1280,80));
	rectangle_bg1.setFillColor(sf::Color(165,255,250));
	rectangle_bg2.setFillColor(sf::Color(10,10,60));
	rectangle_bg3.setFillColor(sf::Color(0,0,0, 75));
	rectangle_menubar.setFillColor(sf::Color::White);
	rectangle_menubar.setPosition(0,640);
	rectangle_bg2.setPosition(640,0);
		//flappy bird
	sf::CircleShape flappy_bird(24);	
	flappy_bird.setFillColor(sf::Color(255,175,0));
	flappy_bird.setPosition(640/4, 640/4);
	vector<Pipe> pipes = {Pipe()};  // vector to hold multiple pipes
		//snake
	sf::RectangleShape snake_block(sf::Vector2f(SNAKE_BLOCK_UNIT, SNAKE_BLOCK_UNIT));
	sf::RectangleShape snake_food_block(sf::Vector2f(SNAKE_BLOCK_UNIT, SNAKE_BLOCK_UNIT));
	sf::RectangleShape snake_buff_block(sf::Vector2f(SNAKE_BLOCK_UNIT, SNAKE_BLOCK_UNIT));
	sf::RectangleShape snake_debuff_block(sf::Vector2f(SNAKE_BLOCK_UNIT, SNAKE_BLOCK_UNIT));
	snake_block.setFillColor(sf::Color::White);
	snake_food_block.setFillColor(sf::Color::Green);
	snake_buff_block.setFillColor(sf::Color::Yellow);
	snake_debuff_block.setFillColor(sf::Color::Red);
			//initialize snake vector of Block's with initial spawn block
	vector<Block> snake = {Block(640 + (static_cast<int>(rand() % 15) * SNAKE_BLOCK_UNIT),static_cast<int>(rand() % 15) * SNAKE_BLOCK_UNIT)};
	Block fruit(1280,720);	
	fruit.sf_block.setFillColor(sf::Color::Red);

	
	// main game loop!! =====================================================
	while (window.isOpen()){
		// handle events
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed){
				window.close();
			}
			//update key bools for later game logic and perform key event logic
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.scancode == sf::Keyboard::Scan::Escape){KEY_ESCAPE = true;}
				if (event.key.scancode == sf::Keyboard::Scan::Space || event.key.scancode == sf::Keyboard::Scan::Enter){KEY_FLAP = true;}
				if (event.key.scancode == sf::Keyboard::Scan::Left || event.key.scancode == sf::Keyboard::Scan::A){KEY_LEFT = true; cout << "left";}
				if (event.key.scancode == sf::Keyboard::Scan::Right || event.key.scancode == sf::Keyboard::Scan::D){KEY_RIGHT = true; cout << "right";}
				if (event.key.scancode == sf::Keyboard::Scan::Down || event.key.scancode == sf::Keyboard::Scan::S){KEY_DOWN = true; cout << "down";}
				if (event.key.scancode == sf::Keyboard::Scan::Up || event.key.scancode == sf::Keyboard::Scan::W){KEY_UP = true; cout << "up";}
				
				if (GAMESTATE == 0) {
					if (KEY_ESCAPE) window.close();
					if (KEY_FLAP) {
						GAMESTATE = 1;
						FLAP_isActive = true;
						SNAKE_isActive = true;
					}
				}
				else if (GAMESTATE == 1){
					if (KEY_ESCAPE) window.close();
					if (FLAP_isActive) if (KEY_FLAP) FLAP_BIRD_YVEL = FLAP_UP_VEL * -1;
					if (SNAKE_isActive) { // 0 = up = {0,-1}, 1 = down = {0, 1}, 2 = left = {-1, 0}, 3 = right = {1, 0}
						if (KEY_UP) {SNAKE_DIR = 0;}
						else if (KEY_DOWN){ SNAKE_DIR = 1;}
						else if (KEY_LEFT){ SNAKE_DIR = 2;}
						else if (KEY_RIGHT){ SNAKE_DIR = 3;}
					}
				}
				else if (GAMESTATE == 2){

				}
				
			}
			if (event.type == sf::Event::KeyReleased) {
				if (event.key.scancode == sf::Keyboard::Scan::Escape){KEY_ESCAPE = false;}
				if (event.key.scancode == sf::Keyboard::Scan::Space || event.key.scancode == sf::Keyboard::Scan::Enter){KEY_FLAP = false;}
				if (event.key.scancode == sf::Keyboard::Scan::Left || sf::Keyboard::Scan::A){KEY_LEFT = false;}
				if (event.key.scancode == sf::Keyboard::Scan::Right || sf::Keyboard::Scan::D){KEY_RIGHT = false;}
				if (event.key.scancode == sf::Keyboard::Scan::Down || sf::Keyboard::Scan::S){KEY_DOWN = false;}
				if (event.key.scancode == sf::Keyboard::Scan::Up || sf::Keyboard::Scan::W){KEY_UP = false;}
				if (GAMESTATE == 0) {

				}
				else if (GAMESTATE == 1){
				
				}
				else if (GAMESTATE == 2){

				}
			}
		}	
		// get ready to draw, perform game logic
		if (GAMESTATE == 0){
			flappy_bird.setPosition(640/4, 640/2);
		}
		else if (GAMESTATE == 1){
			flappyScore_text.setString(to_string(FLAP_SCORE));
			snakeScore_text.setString(to_string(SNAKE_SCORE));
			if (FLAP_isActive) {  // flappy bird logic
				// update the bird's Y position by its Y velocity
				if (FLAP_BIRD_YVEL < 100) FLAP_BIRD_YVEL += FLAP_DOWN_VEL;
				flappy_bird.move(0, FLAP_BIRD_YVEL);	
				// spawn a new pipe after a certain distance
				if (pipes.size() < 2 && pipes.front().piperect.getPosition().x < 365) {
					Pipe pipe;
					pipes.push_back(pipe);
				}
				for (auto& current_pipe : pipes) {
					current_pipe.piperect.move(FLAP_PIPE_SPEED,0);
					current_pipe.gaprect.move(FLAP_PIPE_SPEED,0);
					if(static_cast<int>(flappy_bird.getPosition().x) >= static_cast<int>(current_pipe.piperect.getPosition().x)) FLAP_SCORE += 1;
					if ((flappy_bird.getGlobalBounds().intersects(current_pipe.piperect.getGlobalBounds()) &&!(flappy_bird.getGlobalBounds().intersects(current_pipe.gaprect.getGlobalBounds()))) || flappy_bird.getPosition().y < -96 || flappy_bird.getPosition().y > 664) {
						// game over condiditon
						FLAP_isActive = false;
						SNAKE_isActive = false;
						writeLeaderboard("User ended with flap score of: " + to_string(FLAP_SCORE) + " and a snake score of: " + to_string(SNAKE_SCORE)+ ".\n");
						return 0;
					}
				}
				if (!pipes.empty() && pipes.front().piperect.getPosition().x < -75) {
					pipes.erase(pipes.begin());
				}
			}
			if (SNAKE_isActive) { // snake logic
				SNAKE_FRAMETIMER += 1;
				SNAKE_FRAMETIMER_FRUIT += 1;
				if (!(SNAKE_DIR == -1)) {
					// update snake's direction, assuming it is not invalid (0-2):
					switch(SNAKE_DIR) {
						case 0:
							SNAKE_DX = 0;
							SNAKE_DY = -1;
							break;
						case 1:
							SNAKE_DX = 0;
							SNAKE_DY = 1;
							break;
						case 2:
							SNAKE_DX = -1;
							SNAKE_DY = 0;
							break;
						case 3:
							SNAKE_DX = 1;
							SNAKE_DY = 0;					
							break;
						default:
							SNAKE_DX = 0;
							SNAKE_DY = 0;
					}
					// move the snake, according to its direction
					if (SNAKE_FRAMETIMER >= 30 * SNAKE_SPEED_MULT){ 			
						SNAKE_FRAMETIMER = 0;
						Block current_head = snake.front();

						current_head.moveBlock(SNAKE_DX, SNAKE_DY);
						snake.insert(snake.begin(), current_head);	

						if (snake.front().sf_block.getPosition().x == fruit.sf_block.getPosition().x && snake.front().sf_block.getPosition().y == fruit.sf_block.getPosition().y){
							SNAKE_SCORE += 1;
							fruit.sf_block.setPosition(sf::Vector2f(1280,720));
						}
						else {
							snake.pop_back();
						}
					}
					// spawn a fruit if it is time (every 9 seconds, 420 frames)
					if (SNAKE_FRAMETIMER_FRUIT >= 540 * SNAKE_SPEED_MULT) {
						SNAKE_FRAMETIMER_FRUIT = 0;
						fruit.sf_block.setPosition(sf::Vector2f(640 + (static_cast<int>(rand() % 15) * SNAKE_BLOCK_UNIT), static_cast<int>(rand() % 15) * SNAKE_BLOCK_UNIT));
					}
				}	
			}
		}
		else if (GAMESTATE == 2){

		}


		window.clear(); // clear current frame to render next
		// draw stuff here =====================

		// handle when to draw what
		if (GAMESTATE == 0){
			window.draw(rectangle_bg1);
			window.draw(rectangle_bg2);	
			window.draw(flappy_bird);
			
			window.draw(rectangle_bg3);
			window.draw(title_text);
			window.draw(tutorial_text);
		}
		else if (GAMESTATE == 1){
			window.draw(rectangle_bg1);
			if (FLAP_isActive) {
					//draw flappy bird stuff here
				for(auto& current_pipe : pipes) {
					window.draw(current_pipe.piperect);
					window.draw(current_pipe.gaprect);	
				}
				window.draw(flappy_bird);
			}
			window.draw(rectangle_bg2);	
			if (SNAKE_isActive) {
					// draw snake stuff here
				for(auto& current_block : snake) {
					window.draw(current_block.sf_block);
				}
				window.draw(fruit.sf_block);
			}
			window.draw(rectangle_menubar);
					//draw menu bar stuff here
			window.draw(flappyScore_text);
			window.draw(snakeScore_text);
		}
		else if (GAMESTATE == 2){

		}



	

		// display window to user
		window.display();
	}
	

	// end program
	return 0;
}
