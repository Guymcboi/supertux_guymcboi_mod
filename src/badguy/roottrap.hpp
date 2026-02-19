//  RootTrap
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

#pragma once

#include "object/sticky_object.hpp"

#include "collision/collision_system.hpp"

class RootTrap final : public StickyBadguy
{
public:
  RootTrap(const ReaderMapping& reader);

  virtual void initialize() override;
  virtual void activate() override;
  virtual void active_update(float dt_sec) override;

  virtual HitResponse collision_player(Player& player, const CollisionHit& hit) override;
  static std::string class_name() { return "roottrap"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Root Trap"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual GameObjectClasses get_class_types() const override { return StickyBadguy::get_class_types().add(typeid(RootTrap)); }

  virtual ObjectSettings get_settings() override;
  virtual void kill_fall() override;

  virtual void on_flip(float height) override;

protected:
  virtual std::vector<Direction> get_allowed_directions() const override;

  float get_tile_spawn_pos_offset(const Tile& tile);
  void summon_root();
  bool should_summon_root(const Rectf& bbox);

private:
  bool m_on_step;
  float m_initial_delay;
  float m_spawn_delay;
  bool m_has_stepped;

  State m_state;
  Timer m_spawn_timer;

private:
  RootTrap(const RootTrap&) = delete;
  RootTrap& operator=(const RootTrap&) = delete;
};
