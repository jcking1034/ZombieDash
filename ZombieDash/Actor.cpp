#include "Actor.h"
#include "StudentWorld.h"

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

Actor::Actor(int imageID, int level_x, int level_y, StudentWorld* thing, int startDirection, int depth, bool is_alive, bool block_movement, bool damaged_by_flame, bool damaged_by_vomit, bool block_flames, int multiplier_x, int multiplier_y): GraphObject(imageID, multiplier_x * level_x, multiplier_y * level_y, startDirection, depth) {
    my_world_ptr = thing;
    tick_count = 0;
    alive = is_alive;
    
    m_block_movement = block_movement;
    m_damaged_by_flame = damaged_by_flame;
    m_damaged_by_vomit = damaged_by_vomit;
    m_block_flames = block_flames;
}

bool Actor::collision(int other_x, int other_y) {
    if (!is_alive()) return false;  //  check this
    int x = getX();
    int y = getY();
    
    if (x + SPRITE_WIDTH - 1 >= other_x && x <= other_x + SPRITE_WIDTH - 1) {
        if (y + SPRITE_HEIGHT - 1 >= other_y && y <= other_y + SPRITE_HEIGHT - 1) return true;
    }
    return false;
}

bool Actor::overlap(int other_x, int other_y) {
    int d_x = getX() - other_x;
    int d_y = getY() - other_y;
    
    int result = (d_x * d_x) + (d_y * d_y);
    return result <= 100 && is_alive(); //  check this
}

Person::Person(int imageID, int level_x, int level_y, StudentWorld * thing, bool damaged_by_vomit, int multiplier_x, int multiplier_y): Actor(imageID, level_x, level_y, thing, right, 0, 1, 1, 1, damaged_by_vomit, 0, multiplier_x, multiplier_y) {}

void Person::doSomething() {
    if (!is_alive()) return;
    add_tick();
    do_specific();
}

Human::Human(int imageID, int level_x, int level_y, StudentWorld * thing): Person(imageID, level_x, level_y, thing, 1) {
    infection_status = false;
    infection_count = 0;
}

void Human::do_specific() {
    if (infection_status) infection_count++;
    
    if (infection_count == 500) {
        set_alive(false);
        human_died_specific();
        return;
    }
    
    human_act_specific();
}

Penelope::Penelope(int level_x, int level_y, StudentWorld * thing): Human(IID_PLAYER, level_x, level_y, thing) {
    landmines = 0;
    flamethrower = 0;
    vaccines = 0;
}
//  Actor(IID_PLAYER, level_x, level_y, thing, right, 0, 1, 1)

void Penelope::human_act_specific() {
    StudentWorld* world_ptr = getWorld();
    
    int ch;
    if (world_ptr->getKey(ch)) {
        switch (ch) {
        case KEY_PRESS_SPACE:
            if (flamethrower > 0) {
                flamethrower--;
                world_ptr->playSound(SOUND_PLAYER_FIRE);
                for (int i = 1; i <= 3; i++) {
                    int posx = getX();
                    int posy = getY();
                    
                    int cur_dir = getDirection();
                    switch (cur_dir) {
                    case left:
                        posx -= i * SPRITE_WIDTH;
                        break;
                    case right:
                        posx += i * SPRITE_WIDTH;
                        break;
                    case up:
                        posy += i * SPRITE_HEIGHT;
                        break;
                    case down:
                        posy -= i * SPRITE_HEIGHT;
                        break;
                    }
                    
                    // check if overlap!!!
                    if (!world_ptr->overlap_wall_exit(posx, posy)) {
                        world_ptr->new_flame(posx, posy, cur_dir);
                    } else {
                        break;
                    }
                    
                }
            }
            break;
        case KEY_PRESS_TAB:
            if (landmines > 0) {
                getWorld()->new_landmine(getX(), getY());
                landmines--;
            }
            break;
        case KEY_PRESS_ENTER:
            if (vaccines > 0) {
                clear_infect();
                vaccines--;
            }
            break;
        //check if touching, them update with moveto
        case KEY_PRESS_LEFT: {
            setDirection(left);     // check this
            int destx = getX() - 4;
            if (!world_ptr->collision(destx, getY(), this)) {
                moveTo(destx, getY());
            }
            break;
        }
        case KEY_PRESS_RIGHT: {
            setDirection(right);
            int destx = getX() + 4;
            if (!world_ptr->collision(destx, getY(), this)) {
                moveTo(destx, getY());
            }
            break;
        }
        case KEY_PRESS_UP: {
            setDirection(up);
            int desty = getY() + 4;
            if (!world_ptr->collision(getX(), desty, this)) {
                moveTo(getX(), desty);
            }
            break;
        }
        case KEY_PRESS_DOWN: {
            setDirection(down);
            int desty = getY() - 4;
            if (!world_ptr->collision(getX(), desty, this)) {
                moveTo(getX(), desty);
            }
            break;
        }
        }
    }
}

void Penelope::human_died_specific() {
    StudentWorld* world_ptr = getWorld();
    world_ptr->playSound(SOUND_PLAYER_DIE);
    return;
}

void Penelope::kill() {
    set_alive(false);
    StudentWorld* world_ptr = getWorld();
    world_ptr->playSound(SOUND_PLAYER_DIE);
}

Citizen::Citizen(int level_x, int level_y, StudentWorld * thing): Human(IID_CITIZEN, level_x, level_y, thing) {}

void Citizen::human_died_specific() {
    StudentWorld* world_ptr = getWorld();
    world_ptr->playSound(SOUND_ZOMBIE_BORN);
    world_ptr->increaseScore(-1000);
    world_ptr->new_zombie(getX(), getY());
    world_ptr->remove_citizen();
    return;
}

void Citizen::human_act_specific() {
    if (is_even_tick()) return;
    
    double dist_p = getWorld()->distance(getX(), getY(), getWorld()->player_x(), getWorld()->player_y());
    double dist_z = getWorld()->nearest_zombie(getX(), getY());
    
    if ((dist_p < dist_z || dist_z < 0) && dist_p <= 80) {
        int dest_x = getX();
        int dest_y = getY();
        if (getX() == getWorld()->player_x()) {
            dest_y += ((getY() < getWorld()->player_y()) ? 2 : -2);
            if (!getWorld()->collision(dest_x, dest_y, this) && !getWorld()->collision_with_player(dest_x, dest_y)) {
                setDirection((getY() < getWorld()->player_y()) ? up : down);
                moveTo(dest_x, dest_y);
                return;
            }
        } else if (getY() == getWorld()->player_y()) {
            dest_x += ((getX() < getWorld()->player_x()) ? 2 : -2);
            if (!getWorld()->collision(dest_x, dest_y, this) && !getWorld()->collision_with_player(dest_x, dest_y)) {
                setDirection((getX() < getWorld()->player_x()) ? right : left);
                moveTo(dest_x, dest_y);
                return;
            }
        } else {
            bool move_right = getX() < getWorld()->player_x();
            bool move_up = getY() < getWorld()->player_y();
            
            bool horizontal_or_vert = randInt(0, 1) == 0;
            if (horizontal_or_vert == 0) {
                int dest_x = getX();
                int dest_y = getY();
                if (move_right == 1) dest_x += 2;
                else dest_x -= 2;
                
                if (!getWorld()->collision(dest_x, dest_y, this) && !getWorld()->collision_with_player(dest_x, dest_y)) {
                    setDirection(move_right == 1 ? right : left);
                    moveTo(dest_x, dest_y);
                    return;
                } else {
                    dest_x = getX();
                    dest_y = getY();
                    if (move_up == 1) dest_y += 2;
                    else dest_y -= 2;
                    
                    if (!getWorld()->collision(dest_x, dest_y, this) && !getWorld()->collision_with_player(dest_x, dest_y)) {
                        setDirection(move_up == 1 ? up : down);
                        moveTo(dest_x, dest_y);
                        return;
                    }
                }
            } else {
                int dest_x = getX();
                int dest_y = getY();
                if (move_up == 1) dest_y += 2;
                else dest_y -= 2;
                
                if (!getWorld()->collision(dest_x, dest_y, this) && !getWorld()->collision_with_player(dest_x, dest_y)) {
                    setDirection(move_up == 1 ? up : down);
                    moveTo(dest_x, dest_y);
                    return;
                } else {
                    dest_x = getX();
                    dest_y = getY();
                    if (move_right == 1) dest_x += 2;
                    else dest_x -= 2;
                    
                    if (!getWorld()->collision(dest_x, dest_y, this) && !getWorld()->collision_with_player(dest_x, dest_y)) {
                        setDirection(move_right == 1 ? right : left);
                        moveTo(dest_x, dest_y);
                        return;
                    }
                }
            }
        }
        
    }
    
    // Step 7
    if (dist_z <= 80) {
        bool left_blocked = getWorld()->collision(getX() - 2, getY(), this) || getWorld()->collision_with_player(getX() - 2, getY());
        bool right_blocked = getWorld()->collision(getX() + 2, getY(), this) || getWorld()->collision_with_player(getX() + 2, getY());
        bool up_blocked = getWorld()->collision(getX(), getY() + 2, this) || getWorld()->collision_with_player(getX(), getY() + 2);
        bool down_blocked = getWorld()->collision(getX(), getY() - 2, this) || getWorld()->collision_with_player(getX(), getY() - 2);   //  check this
        
        //  we know that there will always be a zombie here since if there wasn't, dist_z would be -1 and would have been caught in an above if statement
        double left_closest = getWorld()->nearest_zombie(getX() - 2, getY());
        double right_closest = getWorld()->nearest_zombie(getX() + 2, getY());
        double up_closest = getWorld()->nearest_zombie(getX(), getY() + 2);
        double down_closest = getWorld()->nearest_zombie(getX(), getY() - 2);
        
        int best_direction = -1;    //  0=left, 1=right, 2=up, 3=down
        double best_distance = dist_z;
        
        if (left_closest <= dist_z && right_closest <= dist_z && up_closest <= dist_z && down_closest <= dist_z) {
            return;
        } else {
            if (!left_blocked && left_closest > best_distance) best_direction = 0;
            if (!right_blocked && right_closest > best_distance) best_direction = 1;
            if (!up_blocked && up_closest > best_distance) best_direction = 2;
            if (!down_blocked && down_closest > best_distance) best_direction = 3;
            
            switch (best_direction) {
                case 0:
                    setDirection(left);
                    moveTo(getX() - 2, getY());
                    return;
                    break;
                case 1:
                    setDirection(right);
                    moveTo(getX() + 2, getY());
                    return;
                    break;
                case 2:
                    setDirection(up);
                    moveTo(getX(), getY() + 2);
                    return;
                    break;
                case 3:
                    setDirection(down);
                    moveTo(getX(), getY() - 2);
                    return;
                case -1:
                default:
                    return;
                    break;
            }
        }
    }
}

void Citizen::kill() {
    set_alive(false);   //  check this
    getWorld()->increaseScore(-1000);
    getWorld()->remove_citizen();
    getWorld()->playSound(SOUND_CITIZEN_DIE);
}

void Citizen::infect() {
    if (!infected()) {
        getWorld()->playSound(SOUND_CITIZEN_INFECTED);
    }
    Human::infect();
}

Zombie::Zombie(int level_x, int level_y, StudentWorld * thing, int multiplier_x, int multiplier_y): Person(IID_ZOMBIE, level_x, level_y, thing, 0, multiplier_x, multiplier_y) {
    movement_plan_distance = 0;
}

void Zombie::do_specific() {
    if (is_even_tick()) return;
    zombie_act_specific();
}

bool Zombie::try_new_vomit(int x, int y) {
    int vomit_x = x;
    int vomit_y = y;
    switch(getDirection()) {
        case left:
            vomit_x -= SPRITE_WIDTH;    //  check these
            break;
        case right:
            vomit_x += SPRITE_WIDTH;
            break;
        case up:
            vomit_y += SPRITE_HEIGHT;
            break;
        case down:
            vomit_y -= SPRITE_HEIGHT;
            break;
    }
    
    if (getWorld()->close_to_person(vomit_x, vomit_y)) {    //  check if vomit overlaps with wall?
        int random = randInt(1, 3);
        if (random == 1) {
            getWorld()->new_vomit(vomit_x, vomit_y, getDirection());
            getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
            return true;
        }
    }
    return false;
}

Dumb_Zombie::Dumb_Zombie(int level_x, int level_y, StudentWorld * thing, int multiplier_x, int multiplier_y): Zombie(level_x, level_y, thing, multiplier_x, multiplier_y) {}

void Dumb_Zombie::zombie_act_specific() {
    int vomitted = try_new_vomit(getX(), getY());
    if (vomitted) return;
    
    if (get_movement_plan_distance() <= 0) {
        set_movement_plan_distance(randInt(3, 10));
        
        int direction[] = { left, right, up, down };
        setDirection(direction[randInt(0,3)]);
    }
    
    int dest_x = getX();
    int dest_y = getY();
    
    switch (getDirection()) {
        case left:
            dest_x -= 1;    //  check these
            break;
        case right:
            dest_x += 1;
            break;
        case up:
            dest_y += 1;
            break;
        case down:
            dest_y -= 1;
            break;
    }
    
    if (!getWorld()->collision(dest_x, dest_y, this) && !getWorld()->collision_with_player(dest_x, dest_y)) {
        moveTo(dest_x, dest_y);
        set_movement_plan_distance(get_movement_plan_distance() - 1);
    } else {
        set_movement_plan_distance(0);
    }
}

void Dumb_Zombie::kill() {
    getWorld()->increaseScore(1000);    //  check: smart zombie gives 2000 points? will it fall in?
    set_alive(false);
    if (randInt(1,10) == 1) { //
        int drop_x = getX();
        int drop_y = getY();
        switch(randInt(0,3)) {
            case 0:
                drop_x -= SPRITE_WIDTH;
                break;
            case 1:
                drop_x += SPRITE_WIDTH;
                break;
            case 2:
                drop_y += SPRITE_HEIGHT;
                break;
            case 3:
                drop_y -= SPRITE_HEIGHT;
                break;
        }
        if (!getWorld()->overlap_any(drop_x, drop_y)) {
            getWorld()->new_vaccine(drop_x, drop_y);
        }
        
    }
    getWorld()->playSound(SOUND_ZOMBIE_DIE);
}

Smart_Zombie::Smart_Zombie(int level_x, int level_y, StudentWorld * thing, int multiplier_x, int multiplier_y): Zombie(level_x, level_y, thing, multiplier_x, multiplier_y) {}

void Smart_Zombie::zombie_act_specific() {
    int vomitted = try_new_vomit(getX(), getY());
    if (vomitted) return;
    
    if (get_movement_plan_distance() <= 0) {
        set_movement_plan_distance(randInt(3, 10));
    }
    
    double closest_dist = getWorld()->distance(getX(), getY(), getWorld()->player_x(), getWorld()->player_y());
    int closest_dist_x = getWorld()->player_x();
    int closest_dist_y = getWorld()->player_y();
    if (getWorld()->citizens_remain()) {
        if (getWorld()->nearest_citizen(getX(), getY()) < closest_dist) {
            closest_dist = getWorld()->nearest_citizen(getX(), getY());
            closest_dist_x = getWorld()->nearest_citizen_x(getX(), getY());
            closest_dist_y = getWorld()->nearest_citizen_y(getX(), getY());
        }
    }
    
    if (closest_dist > 80) {
        int direction[] = { left, right, up, down };
        setDirection(direction[randInt(0,3)]);
    } else {
        if (getX() == closest_dist_x) {
            setDirection((getY() < closest_dist_y) ? up : down);
        } else if (getY() == closest_dist_y) {
            setDirection((getX() < closest_dist_x) ? right : left);
        } else {
            if (randInt(0, 1) == 1) {
                setDirection((getY() < closest_dist_y) ? up : down);
            } else {
                setDirection((getX() < closest_dist_x) ? right : left);
            }
        }
    }
    
    int dest_x = getX();
    int dest_y = getY();
    switch(getDirection()) {
        case left:
            dest_x--;
            break;
        case right:
            dest_x++;
            break;
        case up:
            dest_y++;
            break;
        case down:
            dest_y--;
            break;
    }
    if (!getWorld()->collision(dest_x, dest_y, this) && !getWorld()->collision_with_player(dest_x, dest_y)) {
        moveTo(dest_x, dest_y);
        set_movement_plan_distance(get_movement_plan_distance() - 1);
    } else {
        set_movement_plan_distance(0);
    }
}

void Smart_Zombie::kill() {
    getWorld()->increaseScore(2000);    //  check: smart zombie gives 2000 points? will it fall in?
    set_alive(false);
    getWorld()->playSound(SOUND_ZOMBIE_DIE);
}

Wall::Wall(int level_x, int level_y, StudentWorld * thing): Actor(IID_WALL, level_x, level_y, thing, right, 0, 1, 1, 0, 0, 1) {}

Activating_Object::Activating_Object(int imageID, int level_x, int level_y, StudentWorld * thing, int direction, int depth, bool damaged_by_flame, bool block_flame, int multiplier_x, int multiplier_y): Actor(imageID, level_x, level_y, thing, direction, depth, 1, 0, damaged_by_flame, 0, block_flame, multiplier_x, multiplier_y) {}

void Activating_Object::doSomething() {
    if (!is_alive()) return;
    specific_do_something();
}

Exit::Exit(int level_x, int level_y, StudentWorld * thing): Activating_Object(IID_EXIT, level_x, level_y, thing, right, 1, 0, 1) {}

void Exit::specific_do_something() {
    if (getWorld()->overlap_citizen_exit(getX(), getY())) {  //  have overlap_citizen kill citizen
        getWorld()->increaseScore(500);
        getWorld()->playSound(SOUND_CITIZEN_SAVED);
    }
    if (getWorld()->overlap_player(getX(), getY()) && !getWorld()->citizens_remain()) {
        getWorld()->level_complete();
    }
}

Pit::Pit(int level_x, int level_y, StudentWorld * thing, int multiplier_x, int multiplier_y): Activating_Object(IID_PIT, level_x, level_y, thing, right, 0, 0, 0, multiplier_x, multiplier_y) {}

void Pit::specific_do_something() {
    getWorld()->overlap_person_pit(getX(), getY());
}

Flame::Flame(int level_x, int level_y, StudentWorld * thing, int direction, int multiplier_x, int multiplier_y): Activating_Object(IID_FLAME, level_x, level_y, thing, direction, 0, 0, 0, multiplier_x, multiplier_y) {}

void Flame::specific_do_something() {
    add_tick();
    if (is_two_tick()) {
        set_alive(false);
        return;
    }
    
    // damage things that overlap: Penelope, citizens, goodies, landmines, zombies (all)
    getWorld()->overlap_any_flame(getX(), getY());
}



Vomit::Vomit(int level_x, int level_y, StudentWorld * thing, int direction, int multiplier_x, int multiplier_y): Activating_Object(IID_VOMIT, level_x, level_y, thing, direction, 0, 0, 0, multiplier_x, multiplier_y) {}

void Vomit::specific_do_something() {
    add_tick();
    if (is_two_tick()) {
        set_alive(false);
        return;
    }
    
    //  do this function
    //  infect Penelope and citizens on overlap
    getWorld()->overlap_person_vomit(getX(), getY());
}

Landmine::Landmine(int level_x, int level_y, StudentWorld * thing, int multiplier_x, int multiplier_y): Activating_Object(IID_LANDMINE, level_x, level_y, thing, right, 1, 1, 0, multiplier_x, multiplier_y) {
    active = false;
}

void Landmine::specific_do_something() {
    if (!active) {
        add_tick();
        if (is_thirty_tick()) active = true;
        return;
    }
    if (getWorld()->overlap_person_landmine(getX(), getY())) {
        kill();
    }
}

void Landmine::kill() {
    set_alive(false);
    getWorld()->playSound(SOUND_LANDMINE_EXPLODE);
    getWorld()->new_flame(getX(), getY(), up);  //  Check this
    
    getWorld()->new_flame(getX() - SPRITE_WIDTH, getY() - SPRITE_HEIGHT, up);
    getWorld()->new_flame(getX() - SPRITE_WIDTH, getY() + SPRITE_HEIGHT, up);
    getWorld()->new_flame(getX() + SPRITE_WIDTH, getY() - SPRITE_HEIGHT, up);
    getWorld()->new_flame(getX() + SPRITE_WIDTH, getY() + SPRITE_HEIGHT, up);
    getWorld()->new_flame(getX(), getY() - SPRITE_HEIGHT, up);
    getWorld()->new_flame(getX(), getY() + SPRITE_HEIGHT, up);
    getWorld()->new_flame(getX() - SPRITE_WIDTH, getY(), up);
    getWorld()->new_flame(getX() + SPRITE_WIDTH, getY(), up);
    
    getWorld()->new_pit(getX(), getY());
}

//  GOODIE
Goodie::Goodie(int imageID, int level_x, int level_y, StudentWorld * thing, int multiplier_x, int multiplier_y): Actor(imageID, level_x, level_y, thing, right, 1, 1, 0, 1, 0, 0, multiplier_x, multiplier_y) {}

void Goodie::doSomething() {
    if (!is_alive()) return;
    if (getWorld()->overlap_player(getX(), getY())) {
        getWorld()->playSound(SOUND_GOT_GOODIE);    //  check this
        set_alive(false);
        grant_specific_goodie_reward();
    }
}

Vaccine_Goodie::Vaccine_Goodie(int level_x, int level_y, StudentWorld * thing, int multiplier_x, int multiplier_y): Goodie(IID_VACCINE_GOODIE, level_x, level_y, thing, multiplier_x, multiplier_y) {}

void Vaccine_Goodie::grant_specific_goodie_reward() {
    getWorld()->increaseScore(50);
    getWorld()->p_add_vaccines();
}

Gas_Can_Goodie::Gas_Can_Goodie(int level_x, int level_y, StudentWorld * thing): Goodie(IID_GAS_CAN_GOODIE, level_x, level_y, thing) {}

void Gas_Can_Goodie::grant_specific_goodie_reward() {
    getWorld()->increaseScore(50);
    getWorld()->p_add_flamethrower();
}

Landmine_Goodie::Landmine_Goodie(int level_x, int level_y, StudentWorld * thing): Goodie(IID_LANDMINE_GOODIE, level_x, level_y, thing) {}

void Landmine_Goodie::grant_specific_goodie_reward() {
    getWorld()->increaseScore(50);
    getWorld()->p_add_landmines();
}
