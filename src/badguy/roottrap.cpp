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

#include "badguy/roottrap.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/root.hpp"
#include "collision/collision_system.hpp"
#include "editor/editor.hpp"
#include "math/aatriangle.hpp"
#include "object/player.hpp"
#include "object/tilemap.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

RootTrap::RootTrap(const ReaderMapping& reader) :
  StickyBadguy(reader, "images/creatures/mole/corrupted/root.sprite", get_allowed_directions()[0], LAYER_TILES-1, COLGROUP_TOUCHABLE),
  m_on_step(false),
  m_initial_delay(),
  m_spawn_delay(),
  m_spawn_timer(),
  m_has_stepped(false)
{
  parse_type(reader);

  reader.get("on-step", m_on_step, false);
  reader.get("sticky", m_sticky, false);
  reader.get("initial-delay", m_initial_delay, 0.0f);
  reader.get("spawn-delay", m_spawn_delay, 3.0f);
  set_colgroup_active(COLGROUP_TOUCHABLE);

  set_action("base", m_dir, 1);

  m_can_glint = false;

  if (!Editor::is_active()) {
    if (m_initial_delay == 0) m_initial_delay = 0.1f;
  }
}

void
RootTrap::initialize()
{
  set_action("base", m_dir);
}

void
RootTrap::activate()
{
  m_spawn_timer.start(m_initial_delay);
}

HitResponse
RootTrap::collision_player(Player&, const CollisionHit& )
{
  if (m_on_step) {
    if (!m_has_stepped) {
      m_has_stepped = true;
      m_spawn_timer.start(m_spawn_delay);
      summon_root();
    }
  }
  return ABORT_MOVE;
}

void
RootTrap::active_update(float dt_sec)
{
  if (m_sticky) {
    sticky_update(dt_sec);
  }

  if (!m_on_step) {
    if (m_spawn_timer.check()) {
      summon_root();
      m_spawn_timer.start(m_spawn_delay);
    }
  } else {
    if (m_spawn_timer.check()) {
      m_has_stepped = false;
    }
  }
}

ObjectSettings
RootTrap::get_settings()
{
  ObjectSettings result = StickyBadguy::get_settings();

  result.add_float(_("Initial delay"), &m_initial_delay, "initial-delay")
    ->set_description(_("Time until the first root is (able to be) spawned after the trap is activated."));
  result.add_bool(_("Trigger on Step"), &m_on_step, "on-step", false)
    ->set_description(_("Only trigger the trap if Tux steps on it."));
  result.add_float(_("Spawn delay"), &m_spawn_delay, "spawn-delay")
    ->set_description(_("Time between consecutive roots."));

  result.reorder({"initial-delay", "spawn-delay", "on-step", "sticky", "direction", "x", "y"});

  return result;
}

void
RootTrap::kill_fall()
{
}

std::vector<Direction>
RootTrap::get_allowed_directions() const
{
  return {  Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT };
}

float
RootTrap::get_tile_spawn_pos_offset(const Tile& tile)
{
  if (!tile.is_slope())
    return 0.f;

  AATriangle slope(tile.get_data());
  int deform = slope.dir & AATriangle::DEFORM_MASK;

  switch (m_dir)
  {
    case Direction::UP:
      if (!slope.is_south())
        return 0.f;

      if (deform == AATriangle::DEFORM_TOP)
        return 16.f;
      else
        return 32.f;

    case Direction::DOWN:
      if (slope.is_south())
        return 0.f;

      if (deform == AATriangle::DEFORM_BOTTOM)
        return -16.f;
      else
        return -32.f;

    case Direction::LEFT:
      if (!slope.is_east())
        return 0.f;

      if (deform == AATriangle::DEFORM_LEFT)
        return 16.f;
      else
        return 32.f;

    case Direction::RIGHT:
      if (slope.is_east())
        return 0.f;

      if (deform == AATriangle::DEFORM_RIGHT)
        return -16.f;
      else
        return -32.f;

    default: assert(false); return 0.f;
  }
}

void
RootTrap::summon_root()
{
  Vector pos;
  float* axis = nullptr;
  switch (m_dir)
  {
    case Direction::UP:
    case Direction::DOWN:
      pos.x = this->get_bbox().get_middle().x;
      axis = &pos.y;
      break;

    case Direction::LEFT:
    case Direction::RIGHT:
      pos.y = this->get_bbox().get_middle().y;
      axis = &pos.x;
      break;

    default: assert(false); break;
  }

  
    switch (m_dir)
    {
      case Direction::UP:
        (*axis) = this->get_bbox().get_bottom() + 16;
        break;
      case Direction::DOWN:
        (*axis) = this->get_bbox().get_top();
        break;
      case Direction::LEFT:
        (*axis) = this->get_bbox().get_right() + 16;
        break;
      case Direction::RIGHT:
        (*axis) = this->get_bbox().get_left();
        break;
      default:
        assert(false);
        break;
    }
   

      bool should_summon = false;
      for (TileMap* tilemap : Sector::get().get_solid_tilemaps())
      {
        const Tile& tile = tilemap->get_tile_at(pos);
        if (tile.is_solid())
        {
          should_summon = true;
          break;
        }
      }

    Sizef size(32.f, 32.f);
    switch (m_dir)
    {
      case Direction::UP: size.height *= 3; break;
      case Direction::DOWN: size.height *= 3; break;
      case Direction::LEFT: size.width *= 3; break;
      case Direction::RIGHT: size.width *= 3; break;
      default: assert(false); break;
    }

    Vector bboxpos = pos;
    switch (m_dir)
    {
      case Direction::DOWN:
        bboxpos.y = std::max(bboxpos.y - (32.f * 3.f), 0.f);
        [[fallthrough]];
      case Direction::UP:
        bboxpos.x -= 16.f;
        break;

      case Direction::RIGHT:
        bboxpos.x = std::max(bboxpos.x - (32.f * 3.f), 0.f);
        [[fallthrough]];
      case Direction::LEFT:
        bboxpos.y -= 16.f;
        break;

      default: assert(false); break;
    }

    // Check if the hitbox of the root is entirely
    // occupied by solid tiles.
    Rectf space(bboxpos, size);
    if (!should_summon_root(space.grown(-1)))
      return;

    Sector::get().add<Root>(pos, m_dir, "images/creatures/mole/corrupted/root.sprite");
}

bool
RootTrap::should_summon_root(const Rectf& bbox)
{
  for (const auto& solids : Sector::get().get_solid_tilemaps())
  {
    if (solids->get_path())
      continue; // Do not support moving tilemaps. Not planned.

    // Test with all tiles in the root's hitbox
    const Rect test_tiles = solids->get_tiles_overlapping(bbox);

    for (int x = test_tiles.left; x < test_tiles.right; ++x)
    {
      for (int y = test_tiles.top; y < test_tiles.bottom; ++y)
      {
        const Tile& tile = solids->get_tile(x, y);

        if (!(tile.get_attributes() & Tile::SOLID))
          goto next_tilemap;
      }
    }

    return true;

next_tilemap:
    ;
  }

  return false;
}

void
RootTrap::on_flip(float height)
{
  BadGuy::on_flip(height);
  if (m_dir == Direction::UP)
  {
    m_dir = Direction::DOWN;
    set_action("base", m_dir, 1);
  }
  else if (m_dir == Direction::DOWN)
  {
    m_dir = Direction::UP;
    set_action("base", m_dir, 1);
  }
  else
  {
    FlipLevelTransformer::transform_flip(m_flip);
  }
}
