/*  IqeFilter plugin for Maverick Model 3D
 *
 * Copyright (c)2018 Zack Middleton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License,or
 * (at your option)any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not,write to the Free Software
 * Foundation,Inc.,59 Temple Place-Suite 330,Boston,MA 02111-1307,
 * USA.
 *
 * See the COPYING file for full license text.
 */

#ifndef __IQEFILTER_H
#define __IQEFILTER_H

#include "modelfilter.h"

#include <string>

class DataDest;

class IqeFilter : public ModelFilter
{
public:

	class IqeOptions : public Options
	{
	public:

		IqeOptions();

		bool m_saveMeshes;
		bool m_savePointsJoint;
		bool m_savePointsAnim;
		bool m_saveSkeleton;
		bool m_saveAnimations;
		std::vector<unsigned int> m_animations;

		virtual void setOptionsFromModel(Model*){}
	};

	Model::ModelErrorE readFile(Model *model, const char *const filename);
	Model::ModelErrorE writeFile(Model *model, const char *const filename, Options &o);

	const char *getWriteTypes(){ return "IQE"; }

	Options *getDefaultOptions(){ return new IqeOptions; };

protected:

	IqeOptions *m_options;

	bool writeLine(DataDest *dst, const char *line,...);
};

#endif // __IQEFILTER_H
