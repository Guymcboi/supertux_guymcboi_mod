//  SuperTux - Experienced Leaf
//  Copyright (C) 2026 e
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "badguy/experiencedleaf.hpp"

#include "audio/sound_manager.hpp"
#include "math/random.hpp"
#include "math/util.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

#include <set>
#include <string>
#include "badguy/mrtree.hpp"
#include "badguy/snail.hpp"
#include "badguy/viciousivy.hpp"
#include "badguy/walkingleaf.hpp"
#include "badguy/igel.hpp"
#include "badguy/jumpy.hpp"

ExperiencedLeaf::ExperiencedLeaf(const ReaderMapping& reader) :
  ViciousIvy(reader, "images/creatures/walkingleaf/walkingleaf.sprite"), m_invincibility_timer(0.0f), m_flight_timer(0.0f), m_is_flying(false), m_has_flown(true), m_rise_strength(), m_rise_time(), m_unfreeze_cooldown(0.0f)
{
  parse_type(reader);

  set_ledge_behavior(LedgeBehavior::FALL);
}

ExperiencedLeaf::ExperiencedLeaf(const Vector& pos, Direction d) :
  ViciousIvy(pos, d, "images/creatures/walkingleaf/walkingleaf.sprite"), m_invincibility_timer(0.0f), m_flight_timer(0.0f), m_is_flying(false), m_has_flown(true), m_rise_strength(), m_rise_time(), m_unfreeze_cooldown(0.0f)
{
  set_ledge_behavior(LedgeBehavior::FALL);
}

GameObjectTypes
ExperiencedLeaf::get_types() const
{
  return {
    { "normal", _("Normal") },
    { "corrupted", _("Corrupted") },
    { "spiny", _("Corrupted (Spiny)") }
  };
}

void
ExperiencedLeaf::active_update(float dt_sec)
{
  if (m_invincibility_timer > 0) {
    m_invincibility_timer -= dt_sec;
    
    if (static_cast<int>(m_invincibility_timer * 10) % 2 == 0) {
      m_sprite->set_color(Color(0.8f, 0.0f, 1.0f, 0.3f));
    } else {
      m_sprite->set_color(Color(1.0f, 1.0f, 1.0f, 1.0f));
    }
    ViciousIvy::active_update(dt_sec);
    return;
  } else {
    m_sprite->set_color(Color(1.0f, 1.0f, 1.0f, 1.0f));
  }

  if (m_unfreeze_cooldown > 0) {
      m_unfreeze_cooldown -= dt_sec;
  }

  if (!on_ground() && !m_is_flying && !m_has_flown && !m_frozen && m_unfreeze_cooldown <= 0) {
      m_is_flying = true;
      m_has_flown = true;
      m_flight_timer = m_rise_time;
  }

  if (m_is_flying && !m_frozen) {
      m_flight_timer -= dt_sec;
      
      m_physic.set_velocity_y(m_rise_strength); 

      if (m_flight_timer <= 0) {
          m_is_flying = false;
      }
  }

  if (on_ground()) {
      m_is_flying = false;
      m_has_flown = false;
      m_flight_timer = 0;
  }
  ViciousIvy::active_update(dt_sec);
}

std::string
ExperiencedLeaf::get_default_sprite_name() const
{
  switch (m_type)
  {
    case SPINY:
      return "images/creatures/vicious_ivy/corrupted/rotten_ivy.sprite";
    case CORRUPTED:
      return "images/creatures/walkingleaf/corrupted/rotten_leaf.sprite";
    default:
      return m_default_sprite_name;
  }
}

void
ExperiencedLeaf::on_type_change(int old_type)
{
  MovingSprite::on_type_change(old_type);

  switch (m_type)
  {
    case NORMAL:
      walk_speed = 100.f;
      m_fall_speed = 35.f;
      m_rise_strength = -50.f;
      m_rise_time = 3.f;
      break;
    case CORRUPTED:
      walk_speed = 85.f;
      m_fall_speed = 80.f;
      m_rise_strength = -40.f;
      m_rise_time = 3.5f;
      break;
    case SPINY:
      walk_speed = 65.f;
      m_fall_speed = 90.f;
      m_rise_strength = -40.f;
      m_rise_time = 4.f;
      break;
    default:
      break;
  }
}

HitResponse
ExperiencedLeaf::collision(MovingObject& other, const CollisionHit& hit)
{
  if (m_invincibility_timer > 0) {
    return ABORT_MOVE; 
  }
  return ViciousIvy::collision(other, hit);
}

void
ExperiencedLeaf::collision_tile(uint32_t tile_attributes)
{
  if (m_invincibility_timer > 0) return;
  ViciousIvy::collision_tile(tile_attributes);
}

bool
ExperiencedLeaf::collision_squished(MovingObject& object)
{
  if (m_invincibility_timer > 0) {
    auto player = dynamic_cast<Player*>(&object);
    if (player) 
      player->bounce(*this);
    return true;
  }

  if (m_frozen)
    return ViciousIvy::collision_squished(object);

  if (m_type == SPINY) {
    auto player = dynamic_cast<Player*>(&object);
    if (player) {
      if (player->is_invincible() || player->m_does_buttjump) {
        Vector pos = get_pos();
        auto& corrupted_leaf = Sector::get().add<ExperiencedLeaf>(pos, m_dir);
        corrupted_leaf.set_type(CORRUPTED);

        if (m_is_glinting)
          corrupted_leaf.m_is_glinting = true;

        corrupted_leaf.m_invincibility_timer = 1.0f;
      
        remove_me();
        player->bounce(*this);
      
       return true;
      } else {
        player->kill(false);
        return true;
      } // valid kill?
    } // is player?
      return false;
  } // spiny?
  return ViciousIvy::collision_squished(object);
} // squished

HitResponse
ExperiencedLeaf::collision_badguy(BadGuy& other, const CollisionHit& hit)
{
  if (this->m_type == NORMAL) {

    bool target_is_corrupted = false;

    if (auto leaf = dynamic_cast<ExperiencedLeaf*>(&other)) {
      if (leaf->m_type == CORRUPTED) target_is_corrupted = true;
    } else if (auto tree = dynamic_cast<MrTree*>(&other)) {
      if (tree->get_type() == 1) {
        target_is_corrupted = true;
      }
    } else if (auto snail = dynamic_cast<Snail*>(&other)) {
      if (snail->get_type() == 1) {
        target_is_corrupted = true;
      }
    } else if (auto ivy = dynamic_cast<ViciousIvy*>(&other)) {
      if (ivy->get_type() == 1) {
        target_is_corrupted = true;
      }
    } else if (auto walking = dynamic_cast<WalkingLeaf*>(&other)) {
      if (walking->get_type() == 1) {
        target_is_corrupted = true;
      }
    } else if (auto igel = dynamic_cast<Igel*>(&other)) {
      if (igel->get_type() == 1) {
        target_is_corrupted = true;
      }
    } else if (auto jumpy = dynamic_cast<Jumpy*>(&other)) {
      if (jumpy->get_type() == 2) {
        target_is_corrupted = true;
      }
    }

    if (target_is_corrupted) {
      this->kill_fall();
      other.kill_fall();
      return ABORT_MOVE;
    }
  }

  if (this->m_type == SPINY) {
    if (auto leaf = dynamic_cast<ExperiencedLeaf*>(&other)) {
      if (leaf->m_type == NORMAL) {
        Vector pos = leaf->get_pos();
      
        auto& new_leaf = Sector::get().add<ExperiencedLeaf>(pos, m_dir);
        new_leaf.set_type(CORRUPTED);

        if (leaf->m_is_glinting)
          new_leaf.m_is_glinting = true;

        leaf->remove_me();

        return ABORT_MOVE;
      }
    } else if (auto tree = dynamic_cast<MrTree*>(&other)) {
      if (tree->get_type() == 0) { 
        
        tree->set_type(1);

        return ABORT_MOVE;
      }
    } else if (auto snail = dynamic_cast<Snail*>(&other)) {
      if (snail->get_type() == 0) {
      
        snail->set_type(1);

        return ABORT_MOVE;
      }
    } else if (auto ivy = dynamic_cast<ViciousIvy*>(&other)) {
      if (ivy->get_type() == 0) {
        
        ivy->set_type(1);

        return ABORT_MOVE;
      }
    } else if (auto walking = dynamic_cast<WalkingLeaf*>(&other)) {
      if (walking->get_type() == 0) {
        
        walking->set_type(1);

        return ABORT_MOVE;
      }
    } else if (auto igel = dynamic_cast<Igel*>(&other)) {
      if (igel->get_type() == 0) {
        
        igel->set_type(1);

        return ABORT_MOVE;
      }
    } else if (auto jumpy = dynamic_cast<Jumpy*>(&other)) {
      if (jumpy->get_type() == 1) {
        
        jumpy->set_type(2);

        return ABORT_MOVE;
      }
    }
  }

  return ViciousIvy::collision_badguy(other, hit);
}

bool
ExperiencedLeaf::is_snipable() const
{
  if (m_type == SPINY) {
    return false;
  } else {
    return true;
  }
}

bool
ExperiencedLeaf::is_flammable() const
{
  if (m_type == SPINY) {
    return false;
  } else {
  return true;
  }
}

void
ExperiencedLeaf::unfreeze(bool )
{
  ViciousIvy::unfreeze(true); 

  m_is_flying = false;
  m_has_flown = true; 
  m_flight_timer = 0;
  
  m_unfreeze_cooldown = 0.5f;
}

std::string
ExperiencedLeaf::get_explosion_sprite() const
{
  switch (m_type)
  {
    case SPINY:
      return "images/particles/rottenleaf.sprite";
    case CORRUPTED:
      return "images/particles/rottenleaf.sprite";
    default:
      return "images/particles/walkingleaf.sprite";
  }
}

void
ExperiencedLeaf::kill_fall()
{
  if (m_invincibility_timer > 0) {
    return; 
  }

  if (m_type == SPINY) {
        Vector pos = get_pos();
        auto& corrupted_leaf = Sector::get().add<ExperiencedLeaf>(pos, m_dir);
        corrupted_leaf.set_type(CORRUPTED);

        if (m_is_glinting)
          corrupted_leaf.m_is_glinting = true;

        corrupted_leaf.m_invincibility_timer = 1.0f;
      
        remove_me();
  } else {
    BadGuy::kill_fall();
  }
}

/* EOF */