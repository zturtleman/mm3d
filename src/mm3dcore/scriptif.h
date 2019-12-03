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


#ifndef __SCRIPTIF_H
#define __SCRIPTIF_H

#include <vector>

#include "model.h"

class MeshRectangle
{
public:

	MeshRectangle(){}
	virtual ~MeshRectangle(){}

	std::vector<int> m_vertices;
	std::vector<int> m_faces;
};

//REMINDER (DAEDALUS)
//
// THERE WAS DOCUMENUTATION FOR EACH API HERE BUT IT WAS
// IN HTML SO I RIPPED IT OUT.
//
/////////////////////////////

extern const char *scriptif_modelGetName(Model *model);

bool scriptif_modelSaveAs(Model *model, const char *f);

extern int  scriptif_modelGetVertexCount(Model *model);

extern int  scriptif_modelGetFaceCount(Model *model);

extern int  scriptif_modelGetGroupCount(Model *model);

extern int  scriptif_modelGetJointCount(Model *model);

extern int  scriptif_modelGetTextureCount(Model *model);

extern int  scriptif_modelGetGroupByName(Model *model, const char *name);

extern const char *scriptif_groupGetName(Model *model, unsigned groupId);

extern int  scriptif_modelGetTextureByName(Model *model, const char *name);

extern const char *scriptif_textureGetName(Model *model, unsigned textureId);

extern const char *scriptif_textureGetFilename(Model *model, unsigned textureId);

extern MeshRectangle *scriptif_modelCreateMeshRectangle(Model *model,
		double x0, double y0, double z0,
		double x1, double y1, double z1,
		double x2, double y2, double z2,
		double x3, double y3, double z3,
		unsigned segments  = 1);

extern int scriptif_modelCreateBoneJoint(Model *model, const char *name,
		double x, double y, double z, double xrot, double yrot, double zrot, int parent);

extern void scriptif_vertexSetCoords(Model *model, unsigned vertexIndex,
		double x, double y, double z);

extern int scriptif_modelAddTexture(Model *model, const char *filename);

extern void scriptif_groupSetTexture(Model *model, unsigned groupId,
		unsigned textureId);

extern void scriptif_faceSetTextureCoords(Model *model, unsigned faceId,
		unsigned vertexIndex, double s, double t);

extern void scriptif_selectedRotate(Model *model,
		double x, double y, double z); // degrees

extern void scriptif_selectedTranslate(Model *model,
		double x, double y, double z);

extern void scriptif_selectedScale(Model *,
		double x, double y, double z);

extern void scriptif_selectedApplyMatrix(Model *model,
		double m0, double m1, double m2, double m3,
		double m4, double m5, double m6, double m7,
		double m8, double m9, double m10, double m11,
		double m12, double m13, double m14, double m15); // 12,13,14 are translation

// For testing only,no documentation
extern void scriptif_selectedDelete(Model *model);

extern void scriptif_selectedWeldVertices(Model *model);

extern void scriptif_selectedInvertNormals(Model *model);

extern int  scriptif_selectedGroupFaces(Model *model, const char *name);

extern void scriptif_selectedAddToGroup(Model *model, int groupId);

extern int scriptif_modelCreateAnimation(Model *model,Model::AnimationModeE mode,
		const char *name, unsigned frameCount, double fps);

extern bool scriptif_setAnimByIndex(Model *model,Model::AnimationModeE mode,
		unsigned anim);

extern bool scriptif_setAnimByName(Model *model,Model::AnimationModeE mode,
		const char *name);

extern void scriptif_animSetFrame(Model *model, unsigned frame);

extern void scriptif_setAnimTime(Model *model, double seconds);

extern int scriptif_animGetCount(Model *model,Model::AnimationModeE mode);

extern const char *scriptif_animGetName(Model *model,Model::AnimationModeE mode, unsigned animIndex);

extern int scriptif_animGetFrameCount(Model *model,Model::AnimationModeE mode, unsigned animIndex);

extern void scriptif_animSetName(Model *model,Model::AnimationModeE mode,
		unsigned animIndex, const char *name);

extern void scriptif_animSetFrameCount(Model *model,Model::AnimationModeE mode,
		unsigned animIndex, unsigned frameCount);

extern void scriptif_animSetFPS(Model *model,Model::AnimationModeE mode,
		unsigned animIndex, double fps);

extern void scriptif_animClearFrame(Model *model,Model::AnimationModeE mode,
		unsigned animIndex, unsigned frame);

extern void scriptif_animCopyFrame(Model *model,Model::AnimationModeE mode,
		unsigned animIndex, unsigned src, unsigned dest);


extern void scriptif_animMove(Model *model,Model::AnimationModeE mode,
		unsigned animIndex1, unsigned animIndex2);

extern int scriptif_animCopy(Model *model,Model::AnimationModeE mode,
		unsigned animIndex, const char *name);

extern int scriptif_animSplit(Model *model,Model::AnimationModeE mode,
		unsigned animIndex, const char *name, unsigned frame);

extern void scriptif_animJoin(Model *model,Model::AnimationModeE mode,
		unsigned anim1, unsigned anim2);

extern int scriptif_animConvertToFrame(Model *model,Model::AnimationModeE mode,
		unsigned animIndex, const char *newName, unsigned frameCount);

extern void scriptif_skelAnimSetKeyframe(Model *model,
		unsigned animIndex, unsigned frame, unsigned joint,bool isRotation,
		double x, double y, double z);

extern void scriptif_skelAnimDeleteKeyframe(Model *model,
		unsigned animIndex, unsigned frame, unsigned joint,bool isRotation);

extern void scriptif_frameAnimSetVertex(Model *model, unsigned animIndex,
		unsigned frame, unsigned v, double x, double y, double z);

extern void scriptif_modelSelectAll(Model *model);

extern void scriptif_modelSelectAllVertices(Model *model);

extern void scriptif_modelSelectAllFaces(Model *model);

extern void scriptif_modelSelectAllGroups(Model *model);

extern void scriptif_modelSelectAllJoints(Model *model);

extern void scriptif_modelSelectVertex(Model *model, int vertex);

extern void scriptif_modelSelectFace(Model *model, int face);

extern void scriptif_modelSelectGroup(Model *model, int group);

extern void scriptif_modelSelectJoint(Model *model, int joint);

extern void scriptif_modelUnselectAll(Model *);

extern void scriptif_logDebug(const char *str);

extern void scriptif_logWarning(const char *str);

extern void scriptif_logError(const char *str);

#endif // __SCRIPTIF_H
