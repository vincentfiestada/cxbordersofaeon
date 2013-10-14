cxbordersofaeon
===============

Machine Problem 2 for my UP D CS11 class. This is a MOBA-inspired battle simulator. The program reads two configuration files, one for warriors and one for items. The user can simulate a battle between two warriors and edit the config files using the program. The full specs are enclosed herein.

What follows is the program specifications copied in verbatim (The original .docx file is available in the Final 1.0 folder

CS 11: MACHINE PROBLEM
BORDERS OF AEON: THE WRATH OF ANDREAS
A multiplayer online battle arena game (MOBA) is typically a game between five heroes from opposing factions.  A new MOBA has been released called the Borders of Aeon: the Wrath of Andreas.  In this game, two opposing teams are each trying to defend their own citadels while attempting to destroy the enemy’s.  In Borders of Aeon: the Wrath of Andreas, the Vision Knights are protecting the Temple on the Dusty Dune with the Frigid Lords defending the Zero Kelvin Spire.  Each faction hires warriors to help them in their efforts.
Each warrior can be classified as smart, dexterous, or tough.  A warrior will always be one of these types.  A Stealth Ninja will always be dexterous.  A Bovine Captain will always be a tough warrior.  In addition, a warrior may have a class from the following: skeptic, mystic, cursed, brute, shredder and vanilla.  One Stealth Ninja can be a mystic, one can be a shredder, and one can be cursed.  One Bovine Captain can be a brute, one can be vanilla, and one can be a mystic.  These classes have advantages and weaknesses over the other classes (except vanilla, which is – well – vanilla).
	Mystic	Cursed	Brute	Shredder	Skeptic
Skeptic	Every fifth attack by the skeptic deals 5% of the energy of the mystic	Every sixth attack by the cursed is evaded by the skeptic			
Mystic		Every fourth attack by the mystic deals 5% of the energy of the mystic to the cursed	Every fifth attack of the mystic increases the mystic’s hit points by the attack damage		
Cursed			Every fourth attack of the brute has half of the damage reflected back to the brute	The shredder’s attack speed is reduced by 20 IAS. 	
Brute				Every fourth attack deals random amount of extra damage capped by 3% of the brute’s hit points	Every sixth attack by the skeptic is met with double the armor of the brute
Shredder	Every sixth attack ignores the mystic’s armor 				Every fifth attack is followed 0.1 seconds with another
Each class can be assigned a power of two for convenience later on in warrior type and class encoding.  Hence, a skeptic is 1, a mystic is 2, a cursed is 4, a brute is 8, a shredder is 16, and a vanilla is 32. 
(Gratuitous flavour text aside, you may check http://www.playdota.com/mechanics for some more information on this project.)
GAME MECHANICS
To implement a one-on-one battle between two warriors, you have to understand the following game mechanics:
WARRIOR ATTRIBUTES
Each warrior has attributes of toughness, dexterity, and smartness.  Each point in toughness increases the hit points of a hero by 19 and increases the hit point regeneration by 0.03 per second.  Each point in dexterity increases the warrior’s armor by 0.14 and increases the warrior’s attack speed by 1.  Each point in smartness increases the warrior’s maximum energy by 13.  (Since there are no mana use and mana burning mechanics in the expected project, mana regeneration – oh, sorry – energy regeneration will be ignored.)
On top of the attribute bonuses, each attribute point increases the damage of a warrior by one if the attribute matches the hero’s type.  For example, a smart warrior’s damage is increased by one for every point in intelligence.
WARRIOR STATISTICS
BASE HIT POINTS
Each warrior’s base hit points is 150. 
BASE ENERGY
Each warrior’s base energy is 0
STARTING AND INCREMENTAL TOUGHNESS, DEXTERITY, AND SMARTNESS
Each warrior will have starting attribute statistics.  Each level adds a certain amount to each of these attributes.  In essence, what you have is a linear function of attributes that also determine other values (read Warrior Attributes).
BASE DAMAGE
The base damage of a warrior is a range from a minimum damage value to a maximum damage value.
BASE ATTACK TIME
This is a statistic that determines the speed at which warriors attack.  Typically at the start, those with high base attack times deal less damage than those who have lower base attack times.
BASE HIT POINT REGENERATION
For the sake of convenience, all warriors in this program will receive a base hit point regeneration of 0.25.
LEVELING
A warrior possesses a level based on experience.  A warrior begins at level 1 and is capped at level 25.
ITEMS AUGMENTATION
Items can augment the warrior’s statistics.  A warrior may carry up to six items that can make them more powerful.
DAMAGE
Each attack will deal a random amount of damage from the base damage of a warrior and all 
HIT POINTS
Hit points are the warrior’s life.  Once a warrior’s life reaches 0 (a warrior may not have negative hit points), the warrior is considered defeated.  Hit points are regenerated using the formula below
HP regenerated per second = Base Hitpoint Regeneration + (Strength * .03) + (Bonus HP Regen from Items and Skills)
ENERGY
Energy is used in actual games for spells.  For this program, it’s only utilized for class advantages.
ARMOR
Armor reduces the damage received by a warrior from the other.
Damage Reduction = ( 0.06 * armor ) / (1 + 0.06 * armor)
ATTACK SPEED
Attack cooldown = BAT/(1 + IAS / 100).
Attack cooldown is the interval between two attacks dealt by a hero.  This is overridden by a shredder’s fifth attack to a skeptic (fixed 0.1 secs).  BAT is a base attack time.  IAS is the sum of all the additional speed given by items and by the dexterity attribute.
PROGRAM REQUIREMENTS
Your program should read a configuration file for warriors and a configuration file for items.  These must be stored in a linked list (since the size may be variable) upon the program’s loading.  The configuration file format for heroes must follow the following format repeated as long as there are heroes to read from the file:
 
Warrior name
Warrior title 
Warrior type and class encoding
Starting toughness
Incremental toughness
Starting dexterity
Incremental dexterity
Starting smartness
Incremental smartness
Base damage minimum
Base damage maximum
Base attack time
Base armor
Relay Chestdrop
Snow Damsel
230
16
1.7
16
1.6
21
2.9
38
44
1.7
1.24
 
The warrior type and class encoding is a binary-format encoding of the possible classes of a warrior for the six rightmost bits and the two leftmost bits representing the type.  The leftmost bits are 01 for tough warriors, 10 for nimble heroes, and 11 for smart heroes.
The configuration file format for the items must follow the following format:
 
Name of item
Additional toughness
Additional dexterity
Additional smartness
Additional armor
Additional attack speed
Additional hit points
Hit point regeneration
Additional energy
Additional damage
Hephaestus Forge
0
0
0
0
80
0
0
0
24
Northrend Skull
25
25
25
0
0
250
0
250
0
 
Your program may then ask for two warriors to pit against each other.  The user will specify the warrior name, the level, the class, and from zero to six items (guaranteeing each step of the way that all input are valid).  They will be made to fight each other until one perishes.  Each game event will be shown along with a “match timer” that shows how far along the head-to-head fight the game events occur.  Assume that each hero’s first attack occurs at 0 secs and HP regeneration occurs at each second.
Finally, your program must be able to add heroes to the configuration file, edit existing heroes, add items, and edit existing items.
