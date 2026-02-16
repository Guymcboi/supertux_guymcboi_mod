//  SuperTux - Bush Igel
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

#include "badguy/bush_igel.hpp"

#include "badguy/root.hpp"
#include "badguy/viciousivy.hpp"
#include "math/aatriangle.hpp"
#include "object/tilemap.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

BushIgel::BushIgel(const ReaderMapping& reader) :
  Igel(reader, "images/creatures/igel/igel.sprite"),
  m_spawn_timer(0.5f),
  m_spawn_length(1.0f)
{
  parse_type(reader);

  set_ledge_behavior(LedgeBehavior::SMART);
}

void
BushIgel::active_update(float dt_sec)
{
 Igel::active_update(dt_sec);

 if (m_frozen) return;

 switch (m_state)
 {
   case STATE_ROLLING:
     if (m_spawn_timer <= 0) {
       switch (m_type)
       {
         case CORRUPTED:
           summon_root();
           break;
         default:
           summon_ivy();
           break;
       }

       m_spawn_timer = m_spawn_length;
     } else {
       m_spawn_timer -= dt_sec;
     }

     break;
   default:
     m_spawn_timer = 0.2f;
     return;
 }
}

std::string
BushIgel::get_default_sprite_name() const
{
  switch (m_type)
  {
    case CORRUPTED:
      return "images/creatures/igel/corrupted/corrupted_igel.sprite";
    default:
      return m_default_sprite_name;
  }
}

void
BushIgel::on_type_change(int old_type)
{
  MovingSprite::on_type_change(old_type);

  switch (m_type)
  {
    case NORMAL:
      m_spawn_length = 0.75f;
      break;
    case CORRUPTED:
      m_spawn_length = 0.2f;
      break;
    default:
      break;
  }
}

void
BushIgel::summon_ivy()
{
  Vector spawn_pos(this->get_x(), this->get_y());
    spawn_pos.x += (m_dir == Direction::LEFT ? 32 : -32);
    spawn_pos.y -= 40;

  Direction spawn_dir;
    spawn_dir = (m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);

  Vector leaf_pos(spawn_pos.x, spawn_pos.y);
  Rectf leaf_bbox(leaf_pos.x, leaf_pos.y, leaf_pos.x + 32, leaf_pos.y + 32);
  // if (Sector::get().is_free_of_movingstatics(leaf_bbox, this)) {
    auto& leaf = Sector::get().add<ViciousIvy>(leaf_bbox.p1(), spawn_dir);
  // }
}

float
BushIgel::get_tile_spawn_pos_offset(const Tile& tile)
{
  if (!tile.is_slope())
    return 0.f;

  AATriangle slope(tile.get_data());
  int deform = slope.dir & AATriangle::DEFORM_MASK;

  if (!slope.is_south())
    return 0.f;

  if (deform == AATriangle::DEFORM_TOP)
    return 16.f;
  else
    return 32.f;
}

void
BushIgel::summon_root()
{
  Vector pos;
  float* axis = nullptr;

  pos.x = this->get_bbox().get_middle().x;
  axis = &pos.y;

   if (this->on_ground())
   {
    (*axis) = this->get_bbox().get_bottom() + 1;

    bool should_summon = false;
    for (TileMap* tilemap : Sector::get().get_solid_tilemaps())
    {
      const Tile& tile = tilemap->get_tile_at(pos);
      if (tile.is_solid())
      {
        should_summon = true;
        break;
      } // tile is solid?
    } // for statement
  } // on ground?

      Sizef size(32.f, 32.f);
      size.height *= 3;

      Vector bboxpos = pos;
      bboxpos.x -= 16.f;

      // Check if the hitbox of the root is entirely
      // occupied by solid tiles.
      Rectf space(bboxpos, size);
      if (should_summon_root(space.grown(-1)))
        Sector::get().add<Root>(pos, Direction::UP, "images/creatures/mole/corrupted/root.sprite");
}

bool
BushIgel::should_summon_root(const Rectf& bbox)
{
  Vector groundpos;
    groundpos.x = this->get_x();
    groundpos.y = this->get_y();

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
