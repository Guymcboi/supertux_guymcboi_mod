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

#include "badguy/bush_igel.hpp"

BushIgel::BushIgel(const ReaderMapping& reader) :
  Igel(reader, "images/creatures/igel/igel.sprite")
{
  parse_type(reader);

  set_ledge_behavior(LedgeBehavior::SMART);
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