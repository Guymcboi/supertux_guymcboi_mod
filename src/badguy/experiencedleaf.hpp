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

#ifndef HEADER_SUPERTUX_BADGUY_EXPERIENCEDLEAF_HPP
#define HEADER_SUPERTUX_BADGUY_EXPERIENCEDLEAF_HPP

#include "badguy/viciousivy.hpp"

class ExperiencedLeaf final : public ViciousIvy
{
public:
  ExperiencedLeaf(const ReaderMapping& reader);
  ExperiencedLeaf(const Vector& pos, Direction d);

  static std::string class_name() { return "experiencedleaf"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Experienced Leaf"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual GameObjectClasses get_class_types() const override { return WalkingBadguy::get_class_types().add(typeid(ExperiencedLeaf)); }

  GameObjectTypes get_types() const override;
  std::string get_default_sprite_name() const override;

  virtual bool is_snipable() const override;
  virtual bool is_flammable() const override;
  virtual void kill_fall() override;
  virtual void unfreeze(bool) override;

  virtual void active_update(float dt_sec) override;

  virtual HitResponse collision(MovingObject& other, const CollisionHit& hit) override;
  virtual void collision_tile(uint32_t tile_attributes) override;
  virtual bool collision_squished(MovingObject& object) override;

public:
  void initialize() override {
    ViciousIvy::initialize();
    m_flight_timer = 0.0f;
    m_is_flying = false;
  }

  void set_type(int new_type) {
    int old_type = m_type;
    m_type = new_type;
    on_type_change(old_type);
  }

protected:
  virtual void on_type_change(int old_type) override;
  virtual std::string get_explosion_sprite() const override;

  virtual HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit) override;

protected:
  float m_invincibility_timer;
  float m_flight_timer;
  bool  m_is_flying;
  bool  m_has_flown;
  float m_rise_strength;
  float m_rise_time;
  float m_unfreeze_cooldown;

protected:
  enum Type {
    NORMAL,
    CORRUPTED,
    SPINY
  };

private:
  ExperiencedLeaf(const ExperiencedLeaf&) = delete;
  ExperiencedLeaf& operator=(const ExperiencedLeaf&) = delete;
};

#endif

/* EOF */