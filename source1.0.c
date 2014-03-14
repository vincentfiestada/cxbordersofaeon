/***************************************

CS11 WFWX2 Machine Problem No. 2
Title: Borders of Aeon

Function: A game inspired by the Multiplayer
			Online Battle Arena (MOBA) genre
			where two warriors with various
			type and item-dependent attributes
			and advantages are pitched against
			each other. The application
			presents a textual "step-by-step"
			overview of the battle.
			
Developer: Vincent Paul Fiestada

Citation: Some code here was copied directly
				from "The C Library Reference Guide" 
				(Eric Huss, 1996), marked below
				by the comment '//(Huss, 1996)'
				
Compatibility Notes: This program has a feature that
							exports html files. The markup
							of those files are based on the
							current HTML5 specs as of 10/06/2013.
							Compatibility with older browsers
							taken into consideration.

***************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define STRINGMAX 80
#define CONFIG_WARRIORS "heroes.cfg"
#define CONFIG_ITEMS "config_items.txt"
/*a bunch of string constants used for decoration*/
#define HR "============================================================"
#define HRL "------------------------------------------------------------"
#define HRW "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
/*App Version*/
#define APP_VERSION "1.0 Final"

#define BASE_HP 150
#define BASE_ENERGY 0
#define BASE_HP_REGEN 0.25

typedef enum{smart, tough, nimble} taxon_type;
typedef enum{mystic, cursed, brute, shredder, skeptic, vanilla} taxon_class;
typedef enum{damage, hpregen} event_type;

/*Type Definition for item 'Object'*/
typedef struct item{
	char name[STRINGMAX];
	
	struct{
		int toughness;
		int dexterity;
		int smartness;
		int armor;
		int attackspeed;
		int hp;
		int hpregen;
		int energy;
		int damage;
	}additional;

	struct item * next;
	struct item * prev;
}item;

typedef struct{
	item * head;
	item * tail;
	item * curr;
	unsigned int size;
}itemlist;

/*Type Definition for Warrior 'Object'*/
typedef struct warrior{
	char name[STRINGMAX];
	char title[STRINGMAX];
	unsigned short int dec_class;
	taxon_type type;
	taxon_class class;
	
	unsigned short int level;
	float hp;
	float hpregen;
	float energy;
	float ias;
	float attackspeed;
	float armor;
	unsigned int attacknum;
	float toughness;
	float dexterity;
	float smartness;
	float augmented_attackvalue;
	
	struct{
		struct{
			float min;
			float max;
		}damage;
		
		float armor;
		float attackvalue;
	}base;
	
	struct{
		float toughness;
		float dexterity;
		float smartness;
	}incremental;
	
	struct{
		item * items[6];
		unsigned short int count;
	}equipment;
	
	struct warrior * next;
	struct warrior * prev;	
}warrior;

typedef struct{
	warrior * head;
	warrior * tail;
	warrior * curr;
	unsigned int size;
}warriorlist;

/*Type Definition for Battle Event
	- this will be used to keep track
	  of damage dealing, HP regen, etc.
*/
typedef struct action{
	event_type type; /*The type of action*/
	warrior * actor; /*The warrior that "does" the action*/
	warrior * object; /*The warrior that recieves the effects of the action*/
	float nextcycle; /*The next time for the event in seconds*/
	float period;
	
	struct{
		struct action * left;
		struct action * right;
	}child;
}action;

typedef struct{
	unsigned int size;
	
	action * root;
	action * curr;
}actiontree;

char * mfgets(char * saveto, float length, FILE * source);
warrior * insert_warriorlist(warrior * node, warriorlist * list);
item * insert_itemlist(item * node, itemlist * list);
warrior * search_warriorlist_name(char * querystr, warrior * startat);
item * search_itemlist_name(char * querystr, item * startat);
unsigned short int inrange(int test, int min, int max);
char * smallcaps(char * str);
void delete_warriorlist(warrior * startat);
void delete_itemlist(item * startat);
void clearscreen();
void printwarrior(warrior * toprint);
action * insert_action(action * node, action * insertat,actiontree * tree);
action * create_action(event_type type, warrior * actor, warrior * object, float nextcycle, float period, actiontree * tree);
float exe_nxt_action(action * node , actiontree * tree, FILE * chronicle, unsigned short int saving_on);
float deal_damage(warrior * attacker, warrior * target, FILE * chronicle, unsigned short int saving_on);
float regen_hp(warrior * target);

int main(){
	FILE * src_warriors;
	FILE * src_items;
	FILE * chronicle;
	char tempstring[STRINGMAX];
	char small_name[STRINGMAX];
	char chronicle_filename[STRINGMAX];
	unsigned short int saving_on = 0;
	char * tempchar;
	warriorlist warriors;
	itemlist items;
	unsigned short int dec_taxon;
	unsigned short int exitstatus = 0;
	unsigned int printbuffer = 0;
	unsigned int iter = 0;
	unsigned int iterx = 0;
	warrior * warrior1 = NULL;
	warrior * warrior2 = NULL;
	warrior * searchresult = NULL;
	item * searchresult_item = NULL;
	unsigned short int tempint;
	taxon_class tempclass;
	actiontree actions;
	float last_exec_cycle;
	float tempfloat;
	time_t today = time(NULL); //(Huss, 1996)
	
	/*Initialize Action Tree*/
	actions.root = NULL;
	actions.size = 0;
	
	/*Initialize Warriors List*/
	warriors.head = NULL;
	warriors.tail = NULL;
	warriors.size = 0;

	/*Initialize Items List*/
	items.head = NULL;
	items.tail = NULL;
	items.size = 0;
	
	clearscreen();
	/*Ask user what to do*/
	printf("\nChoose an option: \n\n [1] Simulate a battle\n [2] Add New Warrior\n [3] Edit a Warrior\n [4] Add New Item\n [5] Edit an Item\n [6] About This Program\n [Other] Exit application\n ");
	tempchar = mfgets(tempstring, 3, stdin); /*Buffer of 2 chars for newline and null characters*/

	switch(*tempchar){
		case '1':
		/*BEGIN SIMULATION CASE*/
		
		/*Open Warriors Configuration File*/
		src_warriors = fopen(CONFIG_WARRIORS,"r");
		/*Check for Error in opening file stream*/
		if(src_warriors==NULL){
			fprintf(stderr,"\nFATAL ERROR!\nFailed to open '%s'. ",CONFIG_WARRIORS);
			exit(0x02);
		}
		
		/*Read all Warriors and Attributes from List*/
		tempchar = mfgets(tempstring, STRINGMAX, src_warriors);
		while(tempchar != NULL){
		
			warriors.curr = malloc(sizeof(warrior));
			strcpy(warriors.curr->name,tempstring);
			mfgets(warriors.curr->title, STRINGMAX, src_warriors);
			dec_taxon = atoi(mfgets(tempstring, STRINGMAX, src_warriors));
			
			/*Convert Decimal Taxonomic Notation to Type and Class*/
			switch(dec_taxon >> 6){ /*Shift 6 bits to the right, we're just interested in the two leftmost bits*/
				case 3:
				warriors.curr->type = smart;
				break;
				case 2:
				warriors.curr->type = nimble;
				break;
				case 1:
				warriors.curr->type = tough;
				break;
				default:
				fprintf(stderr,"\nERROR! Invalid Warrior Type Notation. %s type will default to 'tough'.",warriors.curr->name);
				warriors.curr->type = tough;
				break;
			}
			
			warriors.curr->dec_class = dec_taxon & 63;
			warriors.curr->toughness = atof(mfgets(tempstring, STRINGMAX, src_warriors));
			warriors.curr->incremental.toughness = atof(mfgets(tempstring, STRINGMAX, src_warriors));
			warriors.curr->dexterity = atof(mfgets(tempstring, STRINGMAX, src_warriors));
			warriors.curr->incremental.dexterity = atof(mfgets(tempstring, STRINGMAX, src_warriors));
			warriors.curr->smartness = atof(mfgets(tempstring, STRINGMAX, src_warriors));
			warriors.curr->incremental.smartness = atof(mfgets(tempstring, STRINGMAX, src_warriors));
			warriors.curr->base.damage.min = atof(mfgets(tempstring, STRINGMAX, src_warriors));
			warriors.curr->base.damage.max = atof(mfgets(tempstring, STRINGMAX, src_warriors));
			warriors.curr->attackspeed = atof(mfgets(tempstring, STRINGMAX, src_warriors));
			warriors.curr->base.armor = atof(mfgets(tempstring, STRINGMAX, src_warriors));
			warriors.curr->equipment.count = 0;
			warriors.curr->augmented_attackvalue = 0;
			srand(time(NULL));
			warriors.curr->base.attackvalue = (float)(rand() % (int)((warriors.curr->base.damage.max - warriors.curr->base.damage.min + 1) + warriors.curr->base.damage.min));
			
			/*add node to list*/
			insert_warriorlist(warriors.curr, &warriors);		
			
			tempchar = mfgets(tempstring, STRINGMAX, src_warriors);
		}
		/*Close Warriors File*/
		fclose(src_warriors);
		
		/*Open Items Configuration File*/
		src_items = fopen(CONFIG_ITEMS,"r");
		/*Check for Error in opening file stream*/
		if(src_items==NULL){
			fprintf(stderr,"\nFATAL ERROR!\nFailed to open '%s'. ",CONFIG_ITEMS);
			exit(0x02);
		}
		/*Read all Items and Properties from List*/
		tempchar = mfgets(tempstring, STRINGMAX, src_items);
		while(tempchar != NULL){
		
			items.curr = malloc(sizeof(warrior));
			strcpy(items.curr->name,tempstring);
			items.curr->additional.toughness = atoi(mfgets(tempstring, STRINGMAX, src_items));
			items.curr->additional.dexterity = atoi(mfgets(tempstring, STRINGMAX, src_items));
			items.curr->additional.smartness = atoi(mfgets(tempstring, STRINGMAX, src_items));
			items.curr->additional.armor = atoi(mfgets(tempstring, STRINGMAX, src_items));
			items.curr->additional.attackspeed = atoi(mfgets(tempstring, STRINGMAX, src_items));
			items.curr->additional.hp = atoi(mfgets(tempstring, STRINGMAX, src_items));
			items.curr->additional.hpregen = atoi(mfgets(tempstring, STRINGMAX, src_items));
			items.curr->additional.energy = atoi(mfgets(tempstring, STRINGMAX, src_items));
			items.curr->additional.damage = atoi(mfgets(tempstring, STRINGMAX, src_items));
			
			insert_itemlist(items.curr, &items);		
			
			tempchar = mfgets(tempstring, STRINGMAX, src_items);
		}
		/*Close Items Configuration File*/
		fclose(src_items);
		
		/*Search Warrior List by Name*/
		for(iterx = 1; iterx <=2 ; iterx ++){
		
			printf("\n%s",HR);
			printf("\nCHOOSE TWO WARRIORS FROM THE LIST: [%d of 2]\n",iterx);
			warriors.curr = warriors.head;
			while(warriors.curr != NULL){
				printf("\n -> %s", warriors.curr->name);
				warriors.curr = warriors.curr->next;
			}
			printf("\n\n%s",HR);
			
			do{
				printf("\nEnter Full Name of Warrior %d: ",iterx);
				tempchar = mfgets(tempstring, STRINGMAX, stdin);
				if(tempchar == NULL){
					fprintf(stderr,"\nFATAL ERROR! The program couldn't read your input.");
					exit(0xA3);
				}
				searchresult = NULL;
				searchresult = search_warriorlist_name(smallcaps(tempstring), warriors.head);
				if(searchresult == NULL){
					printf("\nI don't know who that is.\nTry again and enter a valid warrior name this time.");
					printf("\nEnter the full name of the warrior (without the title).\nE.g. \"%s\"\n",warriors.head->name);
				}
			}while(searchresult==NULL);
			
			printf("You selected %s, %s", searchresult->name, searchresult->title);
			
			printf("\n%s",HR);
			do{
				printf("\nEnter Level for Warrior %d [1 to 25]: ",iterx);
				tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
				if(!inrange(tempint,1,25)){
					printf("\nPlease Enter a number from 1 to 25.");
				}
			}while(!inrange(tempint,1,25));
			
			searchresult->level = tempint;
			printf("%s wil be fighting at level %d", searchresult->name, searchresult->level);
			/*63 is 111111, use it to get the six rightmost bits*/
			printf("\n%s",HR);
			do{
				printf("\nChoose a class for Warrior %d: ",iterx);
				printf("\nPossible classes:      ");
				if((searchresult->dec_class&32) == 32){ //100000
					printf("\n[1]Vanilla");
				}
				if((searchresult->dec_class&16) == 16){ //010000
					printf("\n[2]Shredder");
				}
				if((searchresult->dec_class&8) == 8){  //001000
					printf("\n[3]Brute");
				}
				if((searchresult->dec_class&4) == 4){  //000100
					printf("\n[4]Cursed");
				}
				if((searchresult->dec_class&2) == 2){  //000010
					printf("\n[5]Mystic");
				}
				if((searchresult->dec_class&1) == 1){  //000001
					printf("\n[6]Skeptic");
				}
				printf("\n");
				tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
				if(!inrange(tempint,1,6)){
					printf("\nPlease Enter a number based on the list above.");
				}
				else{
				/*Check if the warrior can have that class*/
				switch(tempint){
					case 1:
						if((searchresult->dec_class&32) == 32){
							searchresult->class = vanilla;
						}
						else{
							printf("\n%s cannot be a Vanilla-class.", searchresult->name);
							tempint = -1; //Make tempint invalid to reprompt
						}
					break;
					case 2:
						if((searchresult->dec_class&16) == 16){
							searchresult->class = shredder;
						}
						else{
							printf("\n%s cannot be a Shredder-class.", searchresult->name);
							tempint = -1;
						}
					break;
					case 3:
						if((searchresult->dec_class&8) == 8){
							searchresult->class = brute;
						}
						else{
							printf("\n%s cannot be a Brute-class.", searchresult->name);
							tempint = -1;
						}
					break;
					case 4:
						if((searchresult->dec_class&4) == 4){
							searchresult->class = cursed;
						}
						else{
							printf("\n%s cannot be a Cursed-class.", searchresult->name);
							tempint = -1;
						}
					break;
					case 5:
						if((searchresult->dec_class&2) == 2){
							searchresult->class = mystic;
						}
						else{
							printf("\n%s cannot be a Mystic-class.", searchresult->name);
							tempint = -1;
						}
					break;
					case 6:
						if((searchresult->dec_class&1) == 1){
							searchresult->class = skeptic;
						}
						else{
							printf("\n%s cannot be a Skeptic-class.", searchresult->name);
							tempint = -1;
						}
					break;
					default:
						fprintf(stderr, "\nFATAL ERROR! User Input cannot be understood. Try debugging function inrange(...).\nExiting now.\n");
						exit(0xB1);
					break;
				}
				}
				
			}while(!inrange(tempint,1,6));
			
			printf("\n%s will be fighting as a %s-class warrior.",searchresult->name,
						searchresult->class == vanilla ? "Vanilla" : searchresult->class == shredder ? "Shredder" : searchresult->class == brute ? "Brute" : searchresult->class == cursed ? "Cursed" : searchresult->class == mystic ? "Mystic" : "Skeptic"
					);
					
			printf("\n%s",HR);
			printf("\n AVAILABLE ITEMS: ");
			items.curr = items.head;
			while(items.curr!=NULL){
				printf("\n -> %s",items.curr->name);
				items.curr = items.curr->next;
			}
			printf("\n");
			do{
				printf("\n%s is equipped with %d item%s.", searchresult->name, searchresult->equipment.count,
							searchresult->equipment.count == 1 ? "" : "s" );
				printf("\n\n >> CURRENTLY EQUIPPED WITH << \n");
				if(searchresult->equipment.count == 0){
					printf("\n   No Items");
				}
				else{
					for(iter = 0; iter < searchresult->equipment.count; iter++){
						printf("\n   %s",searchresult->equipment.items[iter]->name);
					}
				}
				printf("\n\nWould you like to select an%s item? [Enter 0 for No] ", searchresult->equipment.count < 1 ? "" : "other");
				mfgets(tempstring, STRINGMAX, stdin); /*STRINGMAX is better than a smaller number since it prevents errors when the user
																	(off chance, though) enters a longer string*/
				if(*tempstring == '0'){
					break;
				}
				searchresult_item = NULL;
				do{
					printf("\nEnter Item Name: ");
					tempchar = mfgets(tempstring, STRINGMAX, stdin);
					if(tempchar == NULL){
						fprintf(stderr,"\nFATAL ERROR! The program couldn't read your input.");
						exit(0xA4);
					}
					searchresult_item = search_itemlist_name(smallcaps(tempstring), items.head);
					if(searchresult_item == NULL){
						printf("\nI can't find that item.\nTry again and enter a valid item name this time.");
					}
					else{
						searchresult->equipment.items[searchresult->equipment.count] = searchresult_item;
						searchresult->equipment.count += 1;
					}
				}while(searchresult_item == NULL);
			}while(searchresult->equipment.count<6);
			
			switch(iterx){
				case 1:
					warrior1 = searchresult;
				break;
				case 2:
					warrior2 = searchresult;
				break;
				default:
					fprintf(stderr,"\nFATAL ERROR! Exiting now.");
					exit(0xA5);
				break;
			}
		}
		
		/*Before beginning the battle simulation, update each warrior's attributes based on item augmentations, etc.*/
		searchresult = NULL; //just make sure it's empty first; not really necessary
		for(iter=0;iter<2;iter++){
			/*Switch which warrior will be applied with the attribute updates*/
			switch(iter){
				case 0:
					searchresult = warrior1;
				break;
				case 1:
					searchresult = warrior2;
				break;
				default:
					fprintf(stderr,"\nFATAL ERROR! Exiting now.");
					exit(0xD1);
			}
			
			/*Apply augmentations to warrior*/
			
			
			/*FROM THE SPEC SHEET: 
			Each warrior has attributes of toughness, dexterity, and smartness.  Each point in toughness increases 
			the hit points of a hero by 19 and increases the hit point regeneration by 0.03 per second.  Each point 
			in dexterity increases the warrior’s armor by 0.14 and increases the warrior’s attack speed by 1.  
			Each point in smartness increases the warrior’s maximum energy by 13.*/
			
			/*Apply incremental toughness, dexterity and smartness, which is how much
			the warrior's corresponding attributes increase per level*/
			searchresult->toughness += searchresult->incremental.toughness * searchresult->level;
			searchresult->smartness += searchresult->incremental.smartness * searchresult->level;
			searchresult->dexterity += searchresult->incremental.dexterity * searchresult->level;
			/*On top of the attribute bonuses, each attribute point increases the damage of a warrior by one if the attribute
			matches the hero’s type*/
			searchresult->base.attackvalue += searchresult->type==nimble ? searchresult->dexterity : searchresult->type==smart ? searchresult->smartness : searchresult->toughness;
			/*Update other attributes that are dependent on previous changes (see specs for game rules)*/
			searchresult->hp = BASE_HP + 19*searchresult->toughness;
			searchresult->energy = BASE_ENERGY + searchresult->smartness;
			searchresult->hpregen = BASE_HP_REGEN + 0.03*searchresult->toughness;
			searchresult->attacknum = 0;
			searchresult->ias = searchresult->dexterity;
			searchresult->armor = searchresult->base.armor + 0.14*searchresult->dexterity;
			
			/*Traverse the array of items and apply the augmentation*/
			for(iterx=0;iterx<searchresult->equipment.count;iterx++){
				searchresult->toughness += searchresult->equipment.items[iterx]->additional.toughness;
				searchresult->dexterity += searchresult->equipment.items[iterx]->additional.dexterity;
				searchresult->smartness += searchresult->equipment.items[iterx]->additional.smartness;
				searchresult->armor += searchresult->equipment.items[iterx]->additional.armor;
				searchresult->ias += searchresult->equipment.items[iterx]->additional.attackspeed;
				searchresult->hp += searchresult->equipment.items[iterx]->additional.hp;
				searchresult->hpregen += searchresult->equipment.items[iterx]->additional.hpregen;
				searchresult->energy += searchresult->equipment.items[iterx]->additional.energy;
				searchresult->augmented_attackvalue += searchresult->equipment.items[iterx]->additional.damage;
			}
			
			/*The shredder’s attack speed is reduced by 20 IAS (if shredder vs cursed). */
			if(warrior1->class==shredder && warrior2->class==cursed){
				warrior1->ias -= 20;
			}
			else if(warrior2->class==shredder && warrior1->class==cursed){
				warrior2->ias -= 20;
			}
			
			/*This (below statement) must be done after the ias has been determined*/
			searchresult->attackspeed = searchresult->attackspeed/(1 + (searchresult->ias / 100));
		}
		
		/*Clear the screen to make the battle simulation chronicle display easier to read*/
		clearscreen();
		/*Begin the battle*/

		/*Print warrior information*/
		printwarrior(warrior1);
		printf("\n\nVS\n");
		printwarrior(warrior2);

		/*Initialize Actions*/
		create_action(damage, warrior1, warrior2, 0.0,warrior1->attackspeed, &actions);
		create_action(damage, warrior2, warrior1, 0.0,warrior2->attackspeed, &actions);
		create_action(hpregen, warrior1, warrior1, 1.0, 1.0 , &actions);
		create_action(hpregen, warrior2, warrior2, 1.0, 1.0 , &actions);
		
		/*Ask the user if he/she wants to save the chronicle as html*/
		printf("\n\n%s\nDo you want to save a chronicle of the simulation as HTML? [Enter 0 for No] ",HRL);
		mfgets(tempstring, STRINGMAX, stdin);
		if(tempstring[0] != '0') saving_on = 1;
		if(saving_on){
			while(1){ //Keep asking until the filename is one that doesn't exist yet
				printf("\nEnter filename to save to (ex. battle_123): ");
				tempchar = mfgets(chronicle_filename, STRINGMAX, stdin);
				if(tempchar == NULL){
					fprintf(stderr, "\nFATAL ERROR! Your input could not be read.\nAborting operation: the chronicle will not be saved.");
					saving_on = 0;
				}
				else if(strcmp(chronicle_filename, "") == 0){
					printf("\nThat filename is invalid. Try again.");
					continue;
				}
				else{
					strcat(chronicle_filename, ".html");
				}
				/*Check first if the file already exists*/
				chronicle = fopen(chronicle_filename, "r");
				fclose(chronicle);
				if(chronicle!=NULL){
					printf("\nThat file already exists!\nOverwriting it might corrupt the contents.\n"
					"Please enter a filename that isn't in use yet.");
				}
				else{
					break;
				}
			}
			printf("\nYour chronicle will be saved as \"%s\"",chronicle_filename);
		}
				
		/*Create a file to write the battle chronicle onto*/
		if(saving_on){
			chronicle = fopen(chronicle_filename, "w"); //C will create a file to write
			fprintf(chronicle,"<!DOCTYPE html>\n<html lang=\"en-US\">\n<head>\n<title>Chronicle: Battle Simulation</title>\n<meta name=\"generator\" content=\"CX Borders of Aeon by Vincent Fiestada\" />\n</head>\n<body>"); //Start writing the html
			fprintf(chronicle,"\n<style type=\"text/css\">\nh1{color:rgb(128,64,0);text-shadow:1px -1px 2px rgba(45,45,45,0.7);}\nh2{color:rgb(221,111,0);}\n.detailed_report{margin-left:65px;padding-left:10px;border-left:1px dashed Black;}\nbody{margin-left:105px;margin-right:105px;font-family:\"Georgia\",serif;background:rgb(254,230,171);}\np{margin-bottom:10px;}\nfooter{font-size:10px;}\n.damage{color:rgb(208,47,47);}\n.hpregen{color:rgb(0,128,64);}\n.stat_tick{color:rgb(0,128,255);}\narticle{text-align:justify;}\n</style>"); 
			/*CSS data for file is included so a separate one doesn't have to written, and so the
				files can be safely exported without destroying the styling.*/
			fprintf(chronicle,"\n<h1>The Epic Duel between %s and %s</h1>\n<h2>A Battle Chronicle from the Imperial Library</h2>", warrior1->name, warrior2->name);
		}
		
		/*>>>>>>>>>>>>>> IMPORTANT: <<<<<<<<<<<<<<
				Starting here, until the simulation is
				finished, the program will write the
				simulation output onto the screen 
				and onto the chronicle if saving_on is on
		*/
		
		printf("\n\nTHE BATTLE BEGINS:");
		if(saving_on){
			strcpy(tempstring,asctime(localtime(&today)));
			tempstring[10] = '\0';
			fprintf(chronicle,"\n<article>\n<p>On %s, in the reign of Andreas, Lord of the Fifteen Realms, Master of the Six Sigmas, and Defender of Namilid, an epic battle took place between the mighty %s, %s and the great %s, %s.</p>",tempstring, warrior1->name, warrior1->title, warrior2->name, warrior2->title);
			//Write down warrior 1's stats in narrative form
			fprintf(chronicle,"<p>");
			if(warrior1->equipment.count > 0){
				fprintf(chronicle,"\nEquipped with %s",warrior1->equipment.items[0]->name);
				if(warrior1->equipment.count > 1){
					fprintf(chronicle," and %d other item%s", warrior1->equipment.count-1, warrior1->equipment.count == 2 ? "":"s");
				}
				fprintf(chronicle,", ");
			}
			fprintf(chronicle,"%s threatened a formidable battle, with as much as %d hit points. The reputation of this warrior is great, as was told by Astavera, Grandmaster of the Sword Kites, known throughout the land as one of the mightiest and %s in the battlefield, fighting this time as a %s-class warrior.",warrior1->name,(int)warrior1->hp,warrior1->type==nimble ? "nimblest" : warrior1->type==smart ? "smartest" : "toughest", warrior1->class == vanilla ? "vanilla" : warrior1->class == shredder ? "shredder" : warrior1->class == brute ? "brute" : warrior1->class == cursed ? "cursed" : warrior1->class == mystic ? "mystic" : "skeptic");
			//Now for warrior 2
			fprintf(chronicle,"</p><p>");
			if(warrior2->equipment.count > 0){
				fprintf(chronicle,"\nWielding the %s",warrior2->equipment.items[0]->name);
				if(warrior2->equipment.count > 1){
					fprintf(chronicle," and %d other instrument%s", warrior2->equipment.count-1, warrior2->equipment.count == 2 ? "":"s");
				}
				fprintf(chronicle,", ");
			}
			fprintf(chronicle,"%s faced the opponent, ready to stand and do honorable duel to the death of all %d hit points. Word of this warrior's skill reached the ten corners of Aeon. Trafalgarix, High Commander of the Army tells that this warrior is one of the greatest and %s in the battlefield, stepping on the arena as a %s-class.",warrior2->name,(int)warrior2->hp,warrior2->type==nimble ? "nimblest" : warrior2->type==smart ? "smartest" : "toughest", warrior2->class == vanilla ? "vanilla" : warrior2->class == shredder ? "shredder" : warrior2->class == brute ? "brute" : warrior2->class == cursed ? "cursed" : warrior2->class == mystic ? "mystic" : "skeptic");
		fprintf(chronicle,"</p>");
		}
		
		printf("\n%s\n -> 00.00 s : %s + %04d vs %04d + %s\n\n%s",
				HR,
		      warrior1->name, (int)warrior1->hp, (int)warrior2->hp, warrior2->name,
				HRW
				);
				
		/*Iterate through the actions, feeding actions from the tree to the exe_nxt_action(...) function
			until one of the warriors loses (gets 0 HP)
		*/
		
		if(saving_on){
			fprintf(chronicle,"\n<p>The battle raged on between the two, with attacks from %s coming almost every %.2f second%s. But %s did not sit idly by, bringing forth attacks every %.2f second%s or so. Of course, when the battle began, it seemed that %s possessed the advantage in terms of base attack power. Below, I have taken note of the highlights of this epic duel. Using my Skill and Ancient Wisdom in the Sacred Way of the Talekeepers, I wrote down this detailed account of every riveting moment in their battle...</p><div class=\"detailed_report\">",warrior2->name, warrior2->attackspeed, warrior2->attackspeed == 1.0 ? "" : "s",warrior1->name, warrior1->attackspeed, warrior1->attackspeed == 1.0 ? "" : "s", warrior1->base.attackvalue > warrior2->base.attackvalue ? warrior1->name : warrior2->name);
		}
		
		while(warrior1->hp > 0 && warrior2->hp > 0){
			last_exec_cycle = exe_nxt_action(actions.root, &actions, chronicle, saving_on);
			printf("\n\n%s\n -> %05.2f s : %s + %04d vs %04d + %s",
				HR, last_exec_cycle,
		      warrior1->name, (int)warrior1->hp, (int)warrior2->hp, warrior2->name
				);
				if(saving_on){
					fprintf(chronicle, "\n<p class=\"stat_tick\">%.2f s into the duel, <b>%s</b> possessed <b>%d</b> hit points, and <b>%s</b> had <b>%d</b> hitpoints.</p>",last_exec_cycle, warrior1->name, (int)warrior1->hp, warrior2->name, (int)warrior2->hp);
				}
		}
		printf("\n\n%s\nTHE BATTLE WAS FINISHED IN %.2f SECONDS",HR, last_exec_cycle);
		if(warrior1->hp == warrior2->hp){
			printf("\n\nIt's a tie between %s and %s", warrior1->name, warrior2->name);
		}
		else{
		printf("\n\nThe Victor is %s", warrior1->hp == 0 ? warrior2->name : warrior1->name);
		}
		
		if(saving_on){
			fprintf(chronicle, "\n</div>\n<p>The entire fiery encounter lasted for no more than %.2f second%s, all of which I faithfully recorded. ", last_exec_cycle, last_exec_cycle == 1.0 ? "" : "s");
			/* >>>>>>>>>>> EASTER EGG >>>>>>>>>>> */
			if(last_exec_cycle < 1){
				fprintf(chronicle,"\nThis extremely short battle interval may be caused by a warping of the Shwartzfeldt tempospatial continuum due to an Edge-Kelvin distortion of the arena space, most likely caused by the high amounts of energy released during the attacks. "); 
			}
			/* >>>>>>>>>>> END >>>>>>>>>> */
			if(warrior1->hp == warrior2->hp){
				fprintf(chronicle,"In the end, it was a <b>stalemate between %s and %s</b>. The Gods smiled upon both of them that day. The honor of both each was spared.</p>", warrior1->name, warrior2->name);
			}
			else{
				fprintf(chronicle,"In the end, <b>%s, %s proved to be the mightier</b>. As is accord with the Mantle of the Gods, the noble warrior proved to be worthy in the battlefield.</p>", warrior1->hp == 0 ? warrior2->name : warrior1->name, warrior1->hp == 0 ? warrior2->title : warrior1->title);
			}
		}
		
		if(saving_on){
			fprintf(chronicle,"\n<p>This chronicle was written in the service of my Lord Andreas, by Vincentius Paulus XXIII, Head Librarian in the Imperial Palace of His Grace.</p>\n</article>\n<hr /><footer>Generated by CX Borders of Aeon, an MP2 Project by a UP Diliman Computer Science Major. Visit <a href=\"http://vincentofearth.wordpress.com/\" target=\"_blank\" >Vincent's Blog</a></footer>\n</body>\n</html>");
			printf("\n\nA chronicle of this simulation was saved as \"%s\"\nin the program directory.",chronicle_filename);
			fclose(chronicle); //Close the chronicle file
		}
		/*END SIMULATION CASE*/
		break;
		
		/*BEGIN WARRIOR ADDING CASE*/
		case '2':
			/*Open warrior config file to append*/
			src_warriors = fopen(CONFIG_WARRIORS,"a");
			printf("\n%s\nENTER INFORMATION FOR NEW WARRIOR: \n",HR);
			while(1){ //Keep doing until break statement kicks in.
				printf("\nName: ");
				mfgets(tempstring, STRINGMAX, stdin);
				strcpy(small_name, tempstring);
				/*Check if that name is already taken*/
				if((search_warriorlist_name(smallcaps(small_name), warriors.head) != NULL) || (strcmp("",small_name) == 0)){
					printf("\nThat warrior name is not valid.\nEither a warrior with that name has already pledged allegiance\nor the name you entered is not allowed.\nPleas enter a different name.");
				}
				else{
					break; //exit out of otherwise infinite loop
				}
			}
			
			fprintf(src_warriors,"\n%s",tempstring);
			printf("\nTitle: ");
			mfgets(tempstring, STRINGMAX, stdin);
			fprintf(src_warriors,"\n%s",tempstring);
			/*Tye and Class: A special 'interface' to make it easier, instead of entering a decimal notation, the user
					can just answer a series of questions and the program converts it to decimal notation for him/her*/
			tempint = 0;
			do{
				printf("\nEnable Skeptic class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 1;
				printf("\nEnable Mystic class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 2;
				printf("\nEnable Cursed class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 4;
				printf("\nEnable Brute class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 8;
				printf("\nEnable Shredder class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 16;
				printf("\nEnable Vanilla class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 32;
				if(tempint<1){	//if tempint < 1, this means that their answer was always no. reprompt
					printf("\nYou need to enable at least 1 class. Try again.");
				}
			}while(tempint<1);
			while(1){ //keep asking until input is valid
				printf("\nChoose a Warrior Type:\n[1]Nimble\n[2]Smart\n[3]Tough\n");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] == '3'){
					tempint += 64;  //01000000
					break; //exit out of otherwise infinite loop
				}
				if(tempstring[0] == '2'){
					tempint += 128; //10000000
					break;
				}
				if(tempstring[0] == '1'){
					tempint += 192; //11000000
					break;
				}
				printf("\nThat input is invalid. Choose from the available options listed.");
			}
			fprintf(src_warriors,"\n%d",tempint);
			printf("\nInitial Toughness: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_warriors,"\n%d",tempint);
			printf("\nIncremental Toughness: ");
			tempfloat = atof(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_warriors,"\n%.2f",tempfloat);
			printf("\nInitial Dexterity: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_warriors,"\n%d",tempint);
			printf("\nIncremental Dexterity: ");
			tempfloat = atof(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_warriors,"\n%.2f",tempfloat);
			printf("\nInitial Smartness: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_warriors,"\n%d",tempint);
			printf("\nIncremental Smartness: ");
			tempfloat = atof(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_warriors,"\n%.2f",tempfloat);
			printf("\nMinimum Base Damage: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_warriors,"\n%d",tempint);
			printf("\nMaximum Base Damage: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_warriors,"\n%d",tempint);
			printf("\nBase Attack Time (seconds): ");
			tempfloat = atof(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_warriors,"\n%.2f",tempfloat);
			printf("\nBase Armor: ");
			tempfloat = atof(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_warriors,"\n%.2f",tempfloat);
						
			/*Close warrior config file*/
			fclose(src_warriors);
		/*END WARRIOR ADDING CASE*/
		break;
		
		case '3':
		/*BEGIN WARRIOR EDITING CASE*/
			/*Open warrior config file to overwrite
				IMPORTANT: Each case must do a separate opening and writing, unless
								one wishes to add yet more members to the warrior struct
								to store static values which will not be changed during
								the course of the simulation.
			*/
			
			/* ---- READ THE FILE FIRST AND STORE IT ---- */
			src_warriors = fopen(CONFIG_WARRIORS,"r");
			/*Read all Warriors and Attributes from List*/
			tempchar = mfgets(tempstring, STRINGMAX, src_warriors);
			while(tempchar != NULL){
			
				warriors.curr = malloc(sizeof(warrior));
				strcpy(warriors.curr->name,tempstring);
				mfgets(warriors.curr->title, STRINGMAX, src_warriors);
				warriors.curr->dec_class = atoi(mfgets(tempstring, STRINGMAX, src_warriors));
				warriors.curr->toughness = atof(mfgets(tempstring, STRINGMAX, src_warriors));
				warriors.curr->incremental.toughness = atof(mfgets(tempstring, STRINGMAX, src_warriors));
				warriors.curr->dexterity = atof(mfgets(tempstring, STRINGMAX, src_warriors));
				warriors.curr->incremental.dexterity = atof(mfgets(tempstring, STRINGMAX, src_warriors));
				warriors.curr->smartness = atof(mfgets(tempstring, STRINGMAX, src_warriors));
				warriors.curr->incremental.smartness = atof(mfgets(tempstring, STRINGMAX, src_warriors));
				warriors.curr->base.damage.min = atof(mfgets(tempstring, STRINGMAX, src_warriors));
				warriors.curr->base.damage.max = atof(mfgets(tempstring, STRINGMAX, src_warriors));
				warriors.curr->attackspeed = atof(mfgets(tempstring, STRINGMAX, src_warriors));
				warriors.curr->base.armor = atof(mfgets(tempstring, STRINGMAX, src_warriors));
				
				/*add node to list*/
				insert_warriorlist(warriors.curr, &warriors);		
				
				tempchar = mfgets(tempstring, STRINGMAX, src_warriors);
			}
			/*Close warrior config file*/
			fclose(src_warriors);
		
			/*Ask the user which warrior to edit*/
			printf("\n%s",HR);
			printf("\nCHOOSE THE WARRIOR YOU WANT TO EDIT:");
			warriors.curr = warriors.head;
			while(warriors.curr != NULL){
				printf("\n <- %s", warriors.curr->name);
				warriors.curr = warriors.curr->next;
			}
			printf("\n\n%s",HR);
			/* --- EDIT THE NODE THEN OVERWRITE THE FILE --- */
			do{
				printf("\nEnter Full Name of Warrior to Edit: ");
				tempchar = mfgets(tempstring, STRINGMAX, stdin);
				if(tempchar == NULL){
					fprintf(stderr,"\nFATAL ERROR! The program couldn't read your input.");
					exit(0xE3);
				}
				searchresult = NULL;
				searchresult = search_warriorlist_name(smallcaps(tempstring), warriors.head);
				if(searchresult == NULL){
					printf("\nI don't know who that is.\nTry again and enter a valid warrior name this time.");
					printf("\nEnter the full name of the warrior (without the title).\nE.g. \"%s\"\n",warriors.head->name);
				}
			}while(searchresult==NULL);
			
			printf("You selected %s", searchresult->name);
			printf("\n\n%s\nEnter New Data for %s.\n",HR,searchresult->name);
			while(1){ //Keep doing until break statement kicks in.
				printf("\nName: ");
				mfgets(tempstring, STRINGMAX, stdin);
				strcpy(small_name, tempstring);
				/*Check if that name is already taken*/
				if(strcmp(smallcaps(small_name), smallcaps(searchresult->name)) == 0){
					break; //exit out of otherwise infinite loop; special case for not changing the name
				}
				else if((search_warriorlist_name(small_name, warriors.head) != NULL) || (strcmp("",small_name) == 0)){
					printf("\nThat warrior name is not valid.\nEither a warrior with that name has already pledged allegiance\nor the name you entered is not allowed.\nPleas enter a different name.");
				}
				else{
					break; //exit out of otherwise infinite loop
				}
			}
			
			strcpy(searchresult->name, tempstring);
			printf("\nTitle (currently \"%s\"): ",searchresult->title);
			mfgets(tempstring, STRINGMAX, stdin);
			strcpy(searchresult->title, tempstring);
			printf("\nType and Class (currently %d in decimal notation):\n",searchresult->dec_class);
			/*Tye and Class: A special 'interface' to make it easier, instead of entering a decimal notation, the user
					can just answer a series of questions and the program converts it to decimal notation for him/her*/
			tempint = 0;
			do{
				printf("\nEnable Skeptic class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 1;
				printf("\nEnable Mystic class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 2;
				printf("\nEnable Cursed class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 4;
				printf("\nEnable Brute class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 8;
				printf("\nEnable Shredder class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 16;
				printf("\nEnable Vanilla class? [Enter 0 for No] ");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] != '0') tempint += 32;
				if(tempint<1){	//if tempint < 1, this means that their answer was always no. reprompt
					printf("\nYou need to enable at least 1 class. Try again.");
				}
			}while(tempint<1);
			while(1){ //keep asking until input is valid
				printf("\nChoose a Warrior Type:\n[1]Nimble\n[2]Smart\n[3]Tough\n");
				mfgets(tempstring, STRINGMAX, stdin);
				if(tempstring[0] == '3'){
					tempint += 64;  //01000000
					break; //exit out of otherwise infinite loop
				}
				if(tempstring[0] == '2'){
					tempint += 128; //10000000
					break;
				}
				if(tempstring[0] == '1'){
					tempint += 192; //11000000
					break;
				}
				printf("\nThat input is invalid. Choose from the available options listed.");
			}
			searchresult->dec_class = tempint;
			printf("\nInitial Toughness (currently %d): ", (int)searchresult->toughness);
			searchresult->toughness = atof(mfgets(tempstring, STRINGMAX, stdin));
			printf("\nIncremental Toughness (currently %.2f): ", searchresult->incremental.toughness);
			searchresult->incremental.toughness = atof(mfgets(tempstring, STRINGMAX, stdin));
			printf("\nInitial Dexterity (currently %d): ", (int)searchresult->dexterity);
			searchresult->dexterity = atof(mfgets(tempstring, STRINGMAX, stdin));
			printf("\nIncremental Dexterity (currently %.2f): ", searchresult->incremental.dexterity);
			searchresult->incremental.dexterity = atof(mfgets(tempstring, STRINGMAX, stdin));
			printf("\nInitial Smartness (currently %d): ", (int)searchresult->smartness);
			searchresult->smartness = atof(mfgets(tempstring, STRINGMAX, stdin));
			printf("\nIncremental Smartness (currently %.2f): ", searchresult->incremental.smartness);
			searchresult->incremental.smartness = atof(mfgets(tempstring, STRINGMAX, stdin));
			printf("\nMinimum Base Damage (currently %d): ", (int)searchresult->base.damage.min);
			searchresult->base.damage.min = atof(mfgets(tempstring, STRINGMAX, stdin));
			printf("\nMaximum Base Damage (currently %d): ", (int)searchresult->base.damage.max);
			searchresult->base.damage.max = atof(mfgets(tempstring, STRINGMAX, stdin));
			printf("\nBase Attack Time (currently %.2f s): ", searchresult->attackspeed);
			searchresult->attackspeed = atof(mfgets(tempstring, STRINGMAX, stdin));
			printf("\nBase Armor (currently %.2f): ", searchresult->base.armor);
			searchresult->base.armor = atof(mfgets(tempstring, STRINGMAX, stdin));			
			
			/*Open the file again, this time for overwriting with the new data*/
			src_warriors = fopen(CONFIG_WARRIORS,"w");
			warriors.curr = warriors.head;
			while(warriors.curr!=NULL){
				/*Write all the data for each warrior*/
				if(warriors.curr!=warriors.head) fprintf(src_warriors, "\n");
				fprintf(src_warriors, "%s",warriors.curr->name);
				fprintf(src_warriors, "\n%s",warriors.curr->title);
				fprintf(src_warriors, "\n%d",warriors.curr->dec_class);
				fprintf(src_warriors, "\n%d",(int)warriors.curr->toughness);
				fprintf(src_warriors, "\n%.2f",warriors.curr->incremental.toughness);
				fprintf(src_warriors, "\n%d",(int)warriors.curr->dexterity);
				fprintf(src_warriors, "\n%.2f",warriors.curr->incremental.dexterity);
				fprintf(src_warriors, "\n%d",(int)warriors.curr->smartness);
				fprintf(src_warriors, "\n%.2f",warriors.curr->incremental.smartness);
				fprintf(src_warriors, "\n%d",(int)warriors.curr->base.damage.min);
				fprintf(src_warriors, "\n%d",(int)warriors.curr->base.damage.max);
				fprintf(src_warriors, "\n%.2f",warriors.curr->attackspeed);
				fprintf(src_warriors, "\n%.2f",warriors.curr->base.armor);
				warriors.curr = warriors.curr->next;
			}
			/*Close the file after the overwrite*/
			fclose(src_warriors);
			
		/*END WARRIOR EDITING CASE*/
		break;
		
		case '4':
			/*BEGIN ITEM ADDING CASE*/
			/*Open item config file to append*/
			src_items = fopen(CONFIG_ITEMS,"a");
			printf("\n%s\nENTER INFORMATION FOR NEW ITEM: \n",HR);
			while(1){ //Keep doing until break statement kicks in.
				printf("\nName: ");
				mfgets(tempstring, STRINGMAX, stdin);
				strcpy(small_name, tempstring);
				/*Check if that name is already taken*/
				if((search_itemlist_name(smallcaps(small_name), items.head) != NULL) || (strcmp("",small_name) == 0)){
					printf("\nThat item name is not valid.\nEither an item with that name already exists\nor the name you entered is not allowed.\nPleas enter a different item name.");
				}
				else{
					break; //exit out of otherwise infinite loop
				}
			}
			/*Ask for and append the data onto the file*/
			fprintf(src_items,"\n%s",tempstring);
			printf("\nAdditional Toughness: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_items,"\n%d",tempint);
			printf("\nAdditional Dexterity: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_items,"\n%d",tempint);
			printf("\nAdditional Smartness: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_items,"\n%d",tempint);
			printf("\nAdditional Armor: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_items,"\n%d",tempint);
			printf("\nAdditional Attack Speed: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_items,"\n%d",tempint);
			printf("\nAdditional Hit Points: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_items,"\n%d",tempint);
			printf("\nAdditional HP Regeneration: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_items,"\n%d",tempint);
			printf("\nAdditional Energy: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_items,"\n%d",tempint);
			printf("\nAdditional Damage: ");
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			fprintf(src_items,"\n%d",tempint);
						
			/*Close item config file*/
			fclose(src_items);
		/*END ITEM ADDING CASE*/
		break;
		
		/*BEGIN ITEM EDITING CASE*/
		case '5':
			/*Open the item config file for reading*/
			src_items = fopen(CONFIG_ITEMS,"r");
			/*Read all Items and Properties from List*/
			tempchar = mfgets(tempstring, STRINGMAX, src_items);
			while(tempchar != NULL){
			
				items.curr = malloc(sizeof(warrior));
				strcpy(items.curr->name,tempstring);
				items.curr->additional.toughness = atoi(mfgets(tempstring, STRINGMAX, src_items));
				items.curr->additional.dexterity = atoi(mfgets(tempstring, STRINGMAX, src_items));
				items.curr->additional.smartness = atoi(mfgets(tempstring, STRINGMAX, src_items));
				items.curr->additional.armor = atoi(mfgets(tempstring, STRINGMAX, src_items));
				items.curr->additional.attackspeed = atoi(mfgets(tempstring, STRINGMAX, src_items));
				items.curr->additional.hp = atoi(mfgets(tempstring, STRINGMAX, src_items));
				items.curr->additional.hpregen = atoi(mfgets(tempstring, STRINGMAX, src_items));
				items.curr->additional.energy = atoi(mfgets(tempstring, STRINGMAX, src_items));
				items.curr->additional.damage = atoi(mfgets(tempstring, STRINGMAX, src_items));
				//Add node to the linked list
				insert_itemlist(items.curr, &items);		
				
				tempchar = mfgets(tempstring, STRINGMAX, src_items);
			}				
			/*Close the file after reading*/
			fclose(src_items);
			
			/*Ask the user which item to edit*/
			printf("\n%s",HR);
			printf("\n CHOOSE THE ITEM YOU WANT TO EDIT: ");
			items.curr = items.head;
			while(items.curr!=NULL){
				printf("\n <- %s",items.curr->name);
				items.curr = items.curr->next;
			}
			printf("\n\n%s",HR);
			while(1){ //Keep doing this until the break statement kicks in
				printf("\nEnter Item Name: ");
				tempchar = mfgets(tempstring, STRINGMAX, stdin);
				if(tempchar == NULL){
					fprintf(stderr,"\nFATAL ERROR! The program couldn't read your input.");
					exit(0xA4);
				}
				searchresult_item = search_itemlist_name(smallcaps(tempstring), items.head);
				if(searchresult_item == NULL){
					printf("\nI can't find that item.\nTry again and enter a valid item name this time.");
				}
				else{
					break; //exit out of the otherwise infinite loop
				}
			}
			printf("\n%s\nEnter New Data for %s\n",HR,searchresult_item->name);
			
			/*Ask for the new data and edit the node*/
			while(1){ //keep doing until break statement
				printf("\nName: ");
				mfgets(tempstring, STRINGMAX, stdin);
				strcpy(small_name,tempstring);
				smallcaps(small_name);
				if(strcmp(small_name, smallcaps(searchresult_item->name)) == 0){
					break; //exit out of the otherwise infinite loop
				}
				else if(strcmp(tempstring, "") == 0 || search_itemlist_name(small_name, items.head) != NULL){
					printf("\nThat name is not valid.\nEither there is already an item with that name\nor it is not allowed.\nTry again.");
				}
				else{
					break; //exit out of the otherwise infinite loop
				}
			}
			strcpy(searchresult_item->name,tempstring);
			printf("\nAdditional Toughness (currently %d): ",(int)searchresult_item->additional.toughness);
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			searchresult_item->additional.toughness = tempint;
			printf("\nAdditional Dexterity (currently %d): ",(int)searchresult_item->additional.dexterity);
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			searchresult_item->additional.dexterity = tempint;
			printf("\nAdditional Smartness (currently %d): ",(int)searchresult_item->additional.smartness);
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			searchresult_item->additional.smartness = tempint;
			printf("\nAdditional Armor (currently %d): ",(int)searchresult_item->additional.armor);
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			searchresult_item->additional.armor = tempint;
			printf("\nAdditional Attack Speed (currently %d): ",(int)searchresult_item->additional.attackspeed);
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			searchresult_item->additional.attackspeed = tempint;
			printf("\nAdditional HP (currently %d): ",(int)searchresult_item->additional.hp);
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			searchresult_item->additional.hp = tempint;
			printf("\nAdditional HP Regen (currently %d): ",(int)searchresult_item->additional.hpregen);
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			searchresult_item->additional.hpregen = tempint;
			printf("\nAdditional Energy (currently %d): ",(int)searchresult_item->additional.energy);
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			searchresult_item->additional.energy = tempint;
			printf("\nAdditional Damage (currently %d): ",(int)searchresult_item->additional.damage);
			tempint = atoi(mfgets(tempstring, STRINGMAX, stdin));
			searchresult_item->additional.damage = tempint;
			/*Open the config file again for overwriting the current data with new one*/
			src_items = fopen(CONFIG_ITEMS,"w");
			items.curr = items.head;
			/*Go through the item linked list and write the data into the config file,
			   overwriting the current data with the new one.*/
			while(items.curr!=NULL){
				if(items.curr!=items.head) fprintf(src_items, "\n");
				fprintf(src_items,"%s",items.curr->name);
				fprintf(src_items,"\n%d", (int)items.curr->additional.toughness);
				fprintf(src_items,"\n%d", (int)items.curr->additional.dexterity);
				fprintf(src_items,"\n%d", (int)items.curr->additional.smartness);
				fprintf(src_items,"\n%d", (int)items.curr->additional.armor);
				fprintf(src_items,"\n%d", (int)items.curr->additional.attackspeed);
				fprintf(src_items,"\n%d", (int)items.curr->additional.hp);
				fprintf(src_items,"\n%d", (int)items.curr->additional.hpregen);
				fprintf(src_items,"\n%d", (int)items.curr->additional.energy);
				fprintf(src_items,"\n%d", (int)items.curr->additional.damage);
				items.curr = items.curr->next;
			}
			/*Close the config file after overwrite*/
			fclose(src_items);
			
		/*END ITEM EDITING CASE*/
		break;
		
		case '6':
		/*DISPLAY PROGRAM INFO*/
		printf("\n\nversion %s\nDeveloped by Vincent Paul Fiestada\n\nCS 11 WFWX2 Machine Problem 2", APP_VERSION);
		break;

		default:
		printf("\nOkay, goodbye.");
		exitstatus = 1;
	}
	

	if(exitstatus == 0){
		printf("\n\n");
		printf("\nPRESS ENTER TO GO BACK TO MAIN MENU ");
		mfgets(tempstring, 3, stdin); //Just a dummy, we only want the user to press enter.
		/*Free up memory before loading it all again*/
		delete_warriorlist(warriors.head);
		delete_itemlist(items.head);
		return main();
	}
	printf("\n\n");
	/*Free up memory before exiting*/
	delete_warriorlist(warriors.head);
	delete_itemlist(items.head);
	return 0;
}

/*<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>
               END OF MAIN
<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>*/

char * smallcaps (char * fc){
	char * curr_letter = fc;
	while(*curr_letter!='\0'){
		if(*curr_letter>=65&&*curr_letter<=90){
			*curr_letter += 32;		//Make small-caps
		}
		curr_letter++;
	}
	return fc;
}

char * mfgets(char * saveto, float length, FILE * source){
	char * newlinecheck;
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
	/*Function mfgets(...) returns pointer to saveto
	after reading string into it.*/
	saveto = fgets(saveto, length, source);
	/*Check for Error in reading string from stream*/
	if(saveto==NULL){
		return NULL;
	}
	/*Remove Rogue newline character*/
	newlinecheck = saveto + strlen(saveto) - 1;
	if(*newlinecheck=='\n'){
		*newlinecheck = '\0';
	}
	return saveto;
}

warrior * insert_warriorlist(warrior * node, warriorlist * list){
	/*A pretty generic function (derived from Machine Exercise 16) that inserts
	the pre-created node into the list. This function DOES NOT INITIALIZE the node
	since there is far too much initialization involved, making for a very long
	and hard-to-read (ugly-to-look-at) argument list.
	Return value is the location of the node in memory*/
	node->next = NULL;
	node->prev = list->tail;
	
	if(list->head==NULL){
		list->head = node;
	}
	else{
		list->tail->next = node;
	}
	list->tail = node;
	list->size += 1;
	
	return node;
}

item * insert_itemlist(item * node, itemlist * list){
	/*Pretty much the same as insert_warriorlist(...) function but with a different pointer type argument and return value
	This one is for adding items (as opposed to warriors) in itemlists (as opposed to warriorlists)*/
	node->next = NULL;
	node->prev = list->tail;
	
	if(list->head==NULL){
		list->head = node;
	}
	else{
		list->tail->next = node;
	}
	list->tail = node;
	list->size += 1;
	
	return node;
}

warrior * search_warriorlist_name(char * querystr, warrior * startat){
	/*Takes a query string as an argument and searches the warriorlist for
	a matching name (case-insensitive);
	Return Value is a pointer to the location of that warrior in memory;
	returns NULL if there are no results after reaching the end
	Theoretically, this function could go wrong, i.e. if there are two warriors
	in the list with the same name, it will return only the first one.
	That's why the user cannot be allowed to add another warrior in the file
	with the same name as an existing warrior (see WARRIOR ADDING CASE above)*/
	char small_name[STRINGMAX];
	
	if(startat == NULL) return NULL; /*If at end, (and still no matches, which is how the function
												  got this far, return NULL to signify that there are no matches*/
	
	strcpy(small_name, startat->name); /*Copy the name into a dummy string, convert it to small caps
	and compare with query string. IMPORTANT: QUERY STRING MUST HAVE BEEN SMALL CAPS IN THE FIRST PLACE.
	Before calling this function, the main(...) function converts the query string to small caps*/
	if(strcmp(smallcaps(small_name), querystr) == 0) return startat; //If equal, return pointer to current location
	
	return search_warriorlist_name(querystr, startat->next); //Recursively check the next item if no match is found
}

item * search_itemlist_name(char * querystr, item * startat){
	/*Same as function search_warriorlist_name(...), but with different pointer type as argument and return value*/
	char small_name[STRINGMAX];
	
	if(startat == NULL) return NULL;
	
	strcpy(small_name, startat->name);
	if(strcmp(smallcaps(small_name), querystr) == 0) return startat;
	
	return search_itemlist_name(querystr, startat->next);
}

/*The following two functions are called just before the program exits, do recursively empty the lists
to prevent garbage from piling up in the system*/
void delete_warriorlist(warrior * startat){
	if(startat == NULL){
		return;
	}
	delete_warriorlist(startat->next);
	/*IMPORTANT: The order of these two statements (above and below this comment) MUST NOT BE REVERSED.
	If reversed, segmentation fault occurs, because the function would be trying to recurse with an argument
	that no longer exists in memory*/
	free(startat);
	return;
}
void delete_itemlist(item * startat){
	/*Same as function delete_warriorlist(...) but (you should know this by now) different pointer type as argument
	and return value*/
	if(startat == NULL){
		return;
	}
	delete_itemlist(startat->next);
	free(startat);
	return;
}

unsigned short int inrange(int test, int min, int max){
	/*Conveniently, this function is here because the task it does is used a number of times (at least 2)
	 It returns a boolean value (1 if the test value is within the acceptable range [min, max] and 0 otherwise)
	*/
	return test>=min && test<=max ? 1 : 0;
}

void clearscreen(){
	/*Clears the terminal screen, also inserting the program banner while we're at it*/
	if(system("cls")){ //If there is an error when calling cls to system (works in Windows) ,...
		system("clear"); //call clear to system instead (works in UNIX, etc.)
	}
	printf("\n\nB O R D E R S   OF   A E O N\n\n");
}
void printwarrior(warrior * toprint){
	/*More like a macro, really:
		prints the minimum amount of warrior information
	*/
	printf("\nLevel %d %s (%s %s) with Attack Cooldown of %.2f s",toprint->level,toprint->name,
			toprint->type == nimble ? "Nimble" : toprint->type == tough ? "Tough" : "Smart",
			toprint->class == vanilla ? "Vanilla" : toprint->class == shredder ? "Shredder" : toprint->class == brute ? 
			"Brute" : toprint->class == cursed ? "Cursed" : toprint->class == mystic ? "Mystic" : "Skeptic",
			toprint->attackspeed 
			/*A series of ternary if statements to determine which string to insert; easy enough to follow once you get the hang
			of it; makes for shorter code, and of course ifs/switches can't be passed as argument to printf*/
			);
}

action * create_action(event_type type, warrior * actor, warrior * object, float nextcycle, float period,actiontree * tree){
	/*Allocates memory for a new action and initializes it, calls a different function to add to tree*/
	tree->curr = malloc(sizeof(action));
	tree->curr->child.left = NULL;
	tree->curr->child.right = NULL;
	tree->curr->type = type;
	tree->curr->actor = actor;
	tree->curr->object = object;
	tree->curr->nextcycle = nextcycle;
	tree->curr->period = period;
	insert_action(tree->curr,tree->root,tree);
	return tree->curr;
}

action * insert_action(action * node, action * insertat,actiontree * tree){
/*This function inserts a node that has already been created into the list
and returns the location of that node in memory (in most case, return value
may be ignored. It is separate from the function that creates and initializes
the node because it is also used to reinsert an action after every execution.
The actions in the binary tree are arranged based on increasing order of 'nextcycle'*/
	action * tempnode;
	
	if(insertat == NULL){
		/*If at a leaf node, insert the node there*/
		/*If there is no root, make it the root*/
		if(tree->root == NULL){
			tree->root = node;
		}
		tree->size+=1;
		return node;
	}
	
	if(insertat->nextcycle > node->nextcycle){
		/*The new action should happen before; insert as left child*/
		tempnode = insert_action(node, insertat->child.left, tree);
		if(insertat->child.left == NULL){
			insertat->child.left = tempnode;
		}
		return tempnode;
	}
	
	if(insertat->nextcycle <= node->nextcycle){ /*Take note of the <= convention
															    This is because insertat would have
																 been in the tree longer, so the new
																 node should come after it*/
																 
		/*The new action should happen after; insert as right child*/
		tempnode = insert_action(node, insertat->child.right, tree);
		if(insertat->child.right == NULL){
			insertat->child.right = tempnode;
		}
		return tempnode;
	}
	
}

float exe_nxt_action(action * node , actiontree * tree, FILE * chronicle, unsigned short int saving_on){
/*Executes the action and changes its nextcycle, then reinserts it into the tree*/
	float last_exec_cycle;
	action * parent = NULL;
	action * xcurr;
	float dmg_dealt = 0;
	
  	/*Find the next action*/
	while(node->child.left != NULL){
		parent = node;
		node = node->child.left;
	}
	
	/*Execute the action*/
	switch(node->type){
		case damage:
			node->actor->attacknum += 1;
			dmg_dealt = deal_damage(node->actor,node->object,chronicle,saving_on);
			printf("\n\n%s\n >> %05.2f s : (%02d) %s deals %d damage to %s",
			       HRL, node->nextcycle, node->actor->attacknum,node->actor->name, 
					 (int)dmg_dealt, node->object->name);
			if(saving_on){
				fprintf(chronicle,"\n<p class=\"damage\">%.2f s into the duel, <b>%s</b>, having attacked <b>%d</b> time%s by my tally, dealt <b>%d</b> damage to <b>%s</b>.</p>",node->nextcycle, node->actor->name,node->actor->attacknum,node->actor->attacknum == 1 ? "" : "s",(int)dmg_dealt,node->object->name);
			}
		break;
		case hpregen:
			printf("\n\n%s\n >> %05.2f s : %s regains %.2f HP", HRL, node->nextcycle, node->object->name, regen_hp(node->object));
			if(saving_on){
				fprintf(chronicle,"\n<p class=\"hpregen\">%.2f s into the duel, <b>%s</b> regained <b>%.2f</b> hit points.</p>",node->nextcycle, node->object->name, regen_hp(node->object));
			}
		break;
		default:
		fprintf(stderr,"\nFATAL ERROR! Action type not recognized. Exiting now.");
		exit(0xC1);
	}
	
	last_exec_cycle = node->nextcycle;
	
	/*Every fifth attack of shredder to skeptic is followed 0.1 seconds with another*/
	if((node->actor->class==shredder) && (node->object->class==skeptic) && (node->actor->attacknum%6 == 0)){
		/* ^ Take note that the attack num has been previously increased by one*/
		if(saving_on){
			fprintf(chronicle,"\n<p>(The Shredder's next attack is accelerated to 0.1 seconds later.)</p>");
		}
		node->nextcycle += 0.1;
	}
	else{
		node->nextcycle += node->period;
	}
	
	/*Remove and then Reinsert node; the left child of its parent will now be its right child*/
	if(parent!=NULL){
		parent->child.left = node->child.right;
	}
	else{
		/*The root must be repositioned, so find the rightmost child of its first child, and that will be the root*/
		tree->root = tree->root->child.right;
	}
	/*Call insert function to reinsert the node based on its new nextcycle member*/
	node->child.left = NULL;
	node->child.right = NULL;
	insert_action(node, tree->root, tree);
	
	return last_exec_cycle;
}

float deal_damage(warrior * attacker, warrior * target, FILE * chronicle, unsigned short int saving_on){
	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	This function takes pointers to two warriors, attacker and target,
	executes the attack, changing the attributes of either, none, or both
	of the warriors, and returns the damage dealt.*/
	float damagedealt = 0;
	float armoreffect = target->armor;
	
	damagedealt = attacker->base.attackvalue + attacker->augmented_attackvalue;
	
	/*Special Warrior Advantages*/
	
	/*Every sixth attack by the skeptic is met with double the armor of the brute*/
	if((attacker->class==skeptic) && (target->class==brute) && (attacker->attacknum%6 == 0)){
		printf("\n\n\%s\n => SPECIAL WARRIOR EFFECT: Skeptic's attack is met with double armor",HRL);
		if(saving_on){
			fprintf(chronicle,"\n<p>(The Skeptic's attack is met with double armor.)</p>");
		}
		armoreffect = 2.0*target->armor;
	}
	/*Every sixth attack ignores the mystic’s armor */
	else if((attacker->class==shredder) && (target->class==mystic) && (attacker->attacknum%6 == 0)){
		printf("\n\n\%s\n => SPECIAL WARRIOR EFFECT: Mystic's armor is disabled",HRL);
		if(saving_on){
			fprintf(chronicle,"\n<p>(The Mystic's armor is temporarily disabled.)</p>");
		}
		armoreffect = 0;
	}
	
	/*Every fifth attack by the skeptic deals 5% of the energy of the mystic*/
	if((attacker->class==skeptic) && (target->class==mystic) && (attacker->attacknum%5 == 0)){
		printf("\n\n\%s\n => SPECIAL WARRIOR EFFECT: Skeptic deals 5%c of Mystic's energy.",HRL, (char)37);
		if(saving_on){
			fprintf(chronicle,"\n<p>(The Skeptic deals 5%c of the Mystic's energy.)</p>",(char)37);
		}
		damagedealt = target->energy * 0.05;
	}
	/*Every sixth attack by the cursed is evaded by the skeptic*/
	if((attacker->class==cursed) && (target->class==skeptic) && (attacker->attacknum%6 == 0)){
		printf("\n\n\%s\n => SPECIAL WARRIOR EFFECT: Skeptic dodges attack of Cursed.",HRL);
		if(saving_on){
			fprintf(chronicle,"\n<p>(The Skeptic skillfully dodges the attack.)</p>");
		}
		damagedealt = 0;
	}
	/*Every fourth attack by the mystic deals 5% of the energy of the mystic to the cursed*/
	if((attacker->class==mystic) && (target->class==cursed) && (attacker->attacknum%4 == 0)){
		printf("\n\n\%s\n => SPECIAL WARRIOR EFFECT: Mystic deals 5%c of energy to Cursed.",HRL, (char)37);
		if(saving_on){
			fprintf(chronicle,"\n<p>(The Mystic deals 5%c of energy as damage.)</p>",(char)37);
		}
		damagedealt = attacker->energy * 0.05;
	}
	/*Every fifth attack of the mystic increases the mystic’s hit points by the attack damage*/
	if((attacker->class==mystic) && (target->class==brute) && (attacker->attacknum%5 == 0)){
		printf("\n\n\%s\n => SPECIAL WARRIOR EFFECT: Mystic regains %d HP.",HRL, (int)damagedealt);
		if(saving_on){
			fprintf(chronicle,"\n<p>(The Mystic uses an enchantment to regain %d HP.)</p>",(int)damagedealt);
		}
		attacker->hp += damagedealt;
	}
	/*Every fourth attack of the brute has half of the damage reflected back to the brute*/
	if((attacker->class==brute) && (target->class=cursed) && (attacker->attacknum%4 == 0)){
		printf("\n\n\%s\n => SPECIAL WARRIOR EFFECT: Brute's attack backfires by 50%c.",HRL, (char)37);
		if(saving_on){
			fprintf(chronicle,"\n<p>(The Brute's attack backfires by half of its magnitude.)</p>");
		}
		attacker->hp -= damagedealt * 0.50;
	}
	/*Every fourth attack deals random amount of extra damage capped by 3% of the brute’s hit points*/
	if((attacker->class==brute) && (target->class==shredder) && (attacker->attacknum%4 == 0)){
		printf("\n\n\%s\n => SPECIAL WARRIOR EFFECT: Brute deals random damage up to 3%c of HP.",HRL, (char)37);
		if(saving_on){
			fprintf(chronicle,"\n<p>(The Brute warrior deals a random damage from 0 to 3%c of the HP)</p>", (char)37);
		}
		damagedealt = rand() % (int)(0 - attacker->hp + 1);
	}
	
	damagedealt += attacker->augmented_attackvalue;
	
	target->hp -= (damagedealt - (( 0.06 * armoreffect ) / (1 + 0.06 * armoreffect)));
	if(target->hp < 0) target->hp = 0; //Negative HP is not allowed
	
	return damagedealt;
}

float regen_hp(warrior * target){
/*Regenerates the warrior's hp, returns amount of hp regenerated*/
	float regeneration = target->hpregen;
	target->hp += regeneration;
	return regeneration;
	return 0;
}