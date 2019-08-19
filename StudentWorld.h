#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include <string>
#include <list>

//class Actor;
//class Penelope;
// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    ~StudentWorld();
    
    void level_complete();
    
    bool collision(int x, int y, Actor * me);   //  blocking of movement
    bool collision_with_player(int x, int y);
    
    bool overlap_wall_exit(int x, int y);   //  object overlap (ex. vomit overlap w person)
    bool overlap_player(int x, int y);
    bool overlap_citizen_exit(int x, int y);
    bool citizens_remain() { return citizens_remaining > 0; }
    void remove_citizen() { citizens_remaining--; }
    bool overlap_person_pit(int x, int y);
    void overlap_any_flame(int x, int y);
    void overlap_person_vomit(int x, int y);
    bool overlap_person_landmine(int x, int y);
    bool close_to_person(int x, int y);
    bool overlap_person_or_wall(int x, int y, Actor * me);
    bool overlap_any(int x, int y);
    
    int player_x() { return player->getX(); }
    int player_y() { return player->getY(); }
    
    double distance(int x1, int y1, int x2, int y2) {
        double dx = x2 - x1;
        double dy = y2 - y1;
        return sqrt((dx * dx) + (dy * dy));
    }
    
    bool overlap(int x1, int y1, int x2, int y2) {  //  just in case, but prefer the one in actor
        return distance(x1, y1, x2, y2) <= 10;
    }
    
    double nearest_zombie(int x, int y);   //  distance from nearest zombie
    double nearest_citizen(int x, int y);
    int nearest_citizen_x(int x, int y);
    int nearest_citizen_y(int x, int y);
    
    void new_flame(int x, int y, int direction);
    void new_zombie(int x, int y);
    void new_vomit(int x, int y, int direction);
    void new_pit(int x, int y);
    void new_landmine(int x, int y);
    void new_vaccine(int x, int y);
    
    void p_add_vaccines();      //  inform StudnetWorld that Penelope gets vaccine (35)
    void p_add_flamethrower(); //  Inform the StudentWorld object that Penelope is to receive 5 charges for her flamethrower.
    void p_add_landmines();     //  Inform the StudentWorld object that Penelope is to receive 2 landmines.
private:
    Penelope * player = nullptr; //   IS THIS OK?
    std::list<Actor *> Actor_list;
    
    bool level_done;
    int citizens_remaining;
};

#endif // STUDENTWORLD_H_
