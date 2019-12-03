/*  MM3D Misfit/Maverick Model 3D
 *
 * Copyright (c)2004-2007 Kevin Worcester
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


#ifndef __MS3DFILTER_H
#define __MS3DFILTER_H

#include "modelfilter.h"
#include "mesh.h"

class Ms3dFilter : public ModelFilter
{
public:
	class Ms3dOptions : public Options
	{
	public:

		Ms3dOptions();

		int m_subVersion;
		uint32_t m_vertexExtra;
		uint32_t m_vertexExtra2;
		uint32_t m_jointColor;

		virtual void setOptionsFromModel(Model *m);
	};

	Ms3dFilter():m_options(){};

	struct _VertexWeight_t
	{
		int boneId;
		int weight;
	}; 
	typedef struct _VertexWeight_t VertexWeightT;
	typedef std::vector<VertexWeightT> VertexWeightList;

	enum _CommentType_e
	{
		CT_GROUP,
		CT_MATERIAL,
		CT_JOINT,
		CT_MODEL,
		CT_MAX,
	};
	typedef enum _CommentType_e CommentTypeE;

	Model::ModelErrorE readFile(Model *model, const char *const filename);
	Model::ModelErrorE writeFile(Model *model, const char *const filename, Options &o);

	Options *getDefaultOptions(){ return new Ms3dOptions; };

	const char *getReadTypes(){ return "MS3D"; }
	const char *getWriteTypes(){ return "MS3D"; }

protected:

	void readString(char *buf, size_t len);

	bool readCommentSection();
	bool readVertexWeightSection();

	bool readVertexWeight(int subVersion, int vertex,
			VertexWeightList &weightList);

	void writeCommentSection();
	void writeVertexWeightSection(const MeshList &ml);
	void writeJointColorSection();

	// The infl_list must be sorted before calling this function
	void writeVertexWeight(int subVersion,
			const infl_list &ilist);

	DataDest	*m_dst;
	DataSource *m_src;

	Model	*m_model;
		
	Ms3dOptions *m_options;

	static char const MAGIC_NUMBER[];
};

#endif // __MS3DFILTER_H
