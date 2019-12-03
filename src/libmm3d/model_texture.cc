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

#include "mm3dtypes.h" //PCH

#include "model.h"
#include "log.h"
#include "texture.h"
#include "texmgr.h"
#include "translate.h"

#include "mm3dport.h"

#ifdef MM3D_EDIT
#include "modelundo.h"
#include "modelstatus.h"
#endif // MM3D_EDIT

int Model::s_glTextures = 0;

bool Model::loadTextures(ContextT context)
{
	LOG_PROFILE();

	DrawingContext *drawContext = nullptr;
	if(context)
	{
		deleteGlTextures(context);
		drawContext = getDrawingContext(context);
		drawContext->m_fileTextures.clear();
		drawContext->m_matTextures.clear();
	}

	for(unsigned t = 0; t<m_materials.size(); t++)
	{
		if(drawContext)
		{
			drawContext->m_matTextures.push_back(-1);
		}

		if(m_materials[t]->m_filename[0] 
			  &&m_materials[t]->m_type==Model::Material::MATTYPE_TEXTURE)
		{
			Texture *tex = TextureManager::getInstance()->getTexture(m_materials[t]->m_filename.c_str());

			if(!tex)
			{
#ifdef MM3D_EDIT
				std::string msg = TRANSLATE("LowLevel","Could not load texture");
				msg += std::string(" ")+m_materials[t]->m_filename;
				model_status(this,StatusError,STATUSTIME_LONG,msg.c_str());
#endif // MM3D_EDIT
				tex = TextureManager::getInstance()->getDefaultTexture(m_materials[t]->m_filename.c_str());
			}
			
			if(tex)
			{
				m_materials[t]->m_textureData = tex;

				if(drawContext)
				{
					int texNum = -1;
					FileTextureMap::iterator it = drawContext->m_fileTextures.find(m_materials[t]->m_filename);
					if(it!=drawContext->m_fileTextures.end())
					{
						texNum = it->second;
					}
					else
					{
						glGenTextures(1,&(m_materials[t]->m_texture));
						s_glTextures++;
						log_debug("GL textures: %d\n",s_glTextures);

						drawContext->m_fileTextures[m_materials[t]->m_filename] =
							m_materials[t]->m_texture;

						texNum = m_materials[t]->m_texture;
					}
					drawContext->m_matTextures[t] = texNum;
				}
				else
				{
					glGenTextures(1,&m_materials[t]->m_texture);
					s_glTextures++;
					log_debug("GL textures: %d\n",s_glTextures);
				}

				log_debug("loaded texture %s as %d\n",tex->m_name.c_str(),m_materials[t]->m_texture);

				//if(context)
				{
					glBindTexture(GL_TEXTURE_2D,m_materials[t]->m_texture);

					//https://github.com/zturtleman/mm3d/issues/85
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
							/*GL_LINEAR*/GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
							GL_LINEAR);

					GLuint format = tex->m_format==Texture::FORMAT_RGBA ? GL_RGBA : GL_RGB;

					//https://github.com/zturtleman/mm3d/issues/85
					glPixelStorei(GL_UNPACK_ALIGNMENT,1);

					gluBuild2DMipmaps(GL_TEXTURE_2D,format, //GLU
							tex->m_width,tex->m_height,
							format,GL_UNSIGNED_BYTE,
							tex->m_data.data());
				}
			}
			else
			{
				log_error("Could not load texture %s\n",m_materials[t]->m_filename.c_str());
			}
		}
	}

	if(drawContext)
	{
		drawContext->m_valid = true;
	}

	texture_manager_do_warning(this);

	return true;
}

#ifdef MM3D_EDIT

int Model::addTexture(Texture *tex)
{
	LOG_PROFILE();

	//REMOVE ME
	if(m_animationMode) //???
	{
		return -1;
	}

	m_changeBits |= AddOther;

	if(tex)
	{
		int num = m_materials.size();

		Material *material = Material::get();

		material->m_name = tex->m_name;
		material->m_type = Material::MATTYPE_TEXTURE;
		material->m_texture = 0;
		material->m_textureData = tex;
		material->m_filename = tex->m_filename;
		for(int m = 0; m<3; m++)
		{
			material->m_ambient[m] = 0.2;
			material->m_diffuse[m] = 0.8;
			material->m_specular[m] = 0.0;
			material->m_emissive[m] = 0.0;
		}
		material->m_ambient[3]  = 1.0;
		material->m_diffuse[3]  = 1.0;
		material->m_specular[3] = 1.0;
		material->m_emissive[3] = 1.0;

		material->m_shininess = 0.0;

		//DrawingContextList m_drawingContexts;
		m_materials.push_back(material);

		MU_AddTexture *undo = new MU_AddTexture();
		undo->addTexture(num,material);
		sendUndo(undo);

		invalidateTextures();
		return num;
	}
	else
	{
		return -1;
	}
}

int Model::addColorMaterial(const char *name)
{
	LOG_PROFILE();
	if(m_animationMode||name==nullptr)
	{
		return -1;
	}

	m_changeBits |= AddOther;

	int num = m_materials.size();

	Material *material = Material::get();

	material->m_name = name;
	material->m_type = Material::MATTYPE_BLANK;
	material->m_texture = 0;
	material->m_textureData = nullptr;
	material->m_filename = "";
	for(int m = 0; m<3; m++)
	{
		material->m_ambient[m] = 0.2;
		material->m_diffuse[m] = 0.8;
		material->m_specular[m] = 0.0;
		material->m_emissive[m] = 0.0;
	}
	material->m_ambient[3]  = 1.0;
	material->m_diffuse[3]  = 1.0;
	material->m_specular[3] = 1.0;
	material->m_emissive[3] = 1.0;

	material->m_shininess = 0.0;

	//DrawingContextList m_drawingContexts;
	m_materials.push_back(material);

	MU_AddTexture *undo = new MU_AddTexture();
	undo->addTexture(num,material);
	sendUndo(undo);

	return num;
}

void Model::deleteTexture(unsigned textureNum)
{
	LOG_PROFILE();

	//REMOVE ME
	if(m_animationMode) return; //???


	for(unsigned g = 0; g<m_groups.size(); g++)
	{
		if(m_groups[g]->m_materialIndex==(signed)textureNum)
		{
			setGroupTextureId(g,-1);
		}
		if(m_groups[g]->m_materialIndex>(signed)textureNum)
		{
			setGroupTextureId(g,m_groups[g]->m_materialIndex-1);
		}
	}

	MU_DeleteTexture *undo = new MU_DeleteTexture();
	undo->deleteTexture(textureNum,m_materials[textureNum]);
	sendUndo(undo);

	removeTexture(textureNum);
}

bool Model::setGroupTextureId(unsigned groupNumber, int textureId)
{
	//REMOVE ME
	if(m_animationMode) return false; //???

	log_debug("assigning texture %d to group %d\n",textureId,groupNumber);

	m_changeBits |= AddOther;

	if(groupNumber>=0&&groupNumber<m_groups.size()&&textureId<(int)m_materials.size())
	{
		m_validBspTree = false;

		MU_SetTexture *undo = new MU_SetTexture();
		undo->setTexture(groupNumber,textureId,
				m_groups[groupNumber]->m_materialIndex);
		sendUndo(undo);

		m_groups[groupNumber]->m_materialIndex = textureId;
		return true;
	}
	else
	{
		return false;
	}
}

bool Model::setTextureCoords(unsigned triangleNumber, unsigned vertexIndex, float s, float t)
{
	//REMOVE ME
	if(m_animationMode) return false; //???

	https://github.com/zturtleman/mm3d/issues/90
	m_changeBits |= MoveGeometry;

	//log_debug("setTextureCoords(%d,%d,%f,%f)\n",triangleNumber,vertexIndex,s,t);

	if(triangleNumber<m_triangles.size()&&vertexIndex<3)
	{
		m_validBspTree = false;

		MU_SetTextureCoords *undo = new MU_SetTextureCoords();
		undo->addTextureCoords(triangleNumber,vertexIndex,s,t,
				m_triangles[triangleNumber]->m_s[vertexIndex],
				m_triangles[triangleNumber]->m_t[vertexIndex]);
		sendUndo(undo,true);

		m_triangles[triangleNumber]->m_s[vertexIndex] = s;
		m_triangles[triangleNumber]->m_t[vertexIndex] = t;

		return true;
	}
	else
	{
		return false;
	}
}

bool Model::setTextureAmbient(unsigned textureId, const float *ambient)
{
	//REMOVE ME
	if(m_animationMode) return false; //???

	if(ambient&&textureId<m_materials.size())
	{
		MU_SetLightProperties *undo = new MU_SetLightProperties();
		undo->setLightProperties(textureId,
				MU_SetLightProperties::LightAmbient,
				ambient,m_materials[textureId]->m_ambient);
		sendUndo(undo);

		for(int t = 0; t<4; t++)
		{
			m_materials[textureId]->m_ambient[t] = ambient[t];
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Model::setTextureDiffuse(unsigned textureId, const float *diffuse)
{
	//REMOVE ME
	if(m_animationMode) return false; //???

	if(diffuse&&textureId<m_materials.size())
	{
		MU_SetLightProperties *undo = new MU_SetLightProperties();
		undo->setLightProperties(textureId,
				MU_SetLightProperties::LightDiffuse,
				diffuse,m_materials[textureId]->m_diffuse);
		sendUndo(undo);

		for(int t = 0; t<4; t++)
		{
			m_materials[textureId]->m_diffuse[t] = diffuse[t];
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Model::setTextureSpecular(unsigned textureId, const float *specular)
{
	//REMOVE ME
	if(m_animationMode) return false; //???

	if(specular&&textureId<m_materials.size())
	{
		MU_SetLightProperties *undo = new MU_SetLightProperties();
		undo->setLightProperties(textureId,
				MU_SetLightProperties::LightSpecular,
				specular,m_materials[textureId]->m_specular);
		sendUndo(undo);

		for(int t = 0; t<4; t++)
		{
			m_materials[textureId]->m_specular[t] = specular[t];
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Model::setTextureEmissive(unsigned textureId, const float *emissive)
{
	//REMOVE ME
	if(m_animationMode) return false; //???

	if(emissive&&textureId<m_materials.size())
	{
		MU_SetLightProperties *undo = new MU_SetLightProperties();
		undo->setLightProperties(textureId,
				MU_SetLightProperties::LightEmissive,
				emissive,m_materials[textureId]->m_emissive);
		sendUndo(undo);

		for(int t = 0; t<4; t++)
		{
			m_materials[textureId]->m_emissive[t] = emissive[t];
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Model::setTextureShininess(unsigned textureId, float shininess)
{
	//REMOVE ME
	if(m_animationMode) return false; //???

	if(textureId<m_materials.size())
	{
		MU_SetShininess *undo = new MU_SetShininess();
		undo->setShininess(textureId,shininess,m_materials[textureId]->m_shininess);
		sendUndo(undo);

		m_materials[textureId]->m_shininess = shininess;
		return true;
	}
	else
	{
		return false;
	}
}

void Model::setTextureName(unsigned textureId, const char *name)
{
	//REMOVE ME
	if(m_animationMode) return; //???

	if(name&&textureId<m_materials.size())
	{
		MU_SetTextureName *undo = new MU_SetTextureName();
		undo->setTextureName(textureId,name,m_materials[textureId]->m_name.c_str());
		sendUndo(undo);

		m_materials[textureId]->m_name = name;
	}
}

void Model::setMaterialTexture(unsigned textureId,Texture *tex)
{
	//REMOVE ME
	if(m_animationMode) return; //???

	if(tex==nullptr)
	{
		removeMaterialTexture(textureId);
	}
	else if(textureId<m_materials.size())
	{
		MU_SetMaterialTexture *undo = new MU_SetMaterialTexture();
		undo->setMaterialTexture(textureId,tex,m_materials[textureId]->m_textureData);
		sendUndo(undo);

		m_materials[textureId]->m_textureData = tex;
		m_materials[textureId]->m_filename = tex->m_filename;
		m_materials[textureId]->m_type = Material::MATTYPE_TEXTURE;
		invalidateTextures();
	}
}

void Model::removeMaterialTexture(unsigned textureId)
{
	//REMOVE ME
	if(m_animationMode) return; //???

	if(textureId<m_materials.size())
	{
		if(m_materials[textureId]->m_type==Material::MATTYPE_TEXTURE)
		{
			MU_SetMaterialTexture *undo = new MU_SetMaterialTexture();
			undo->setMaterialTexture(textureId,nullptr,m_materials[textureId]->m_textureData);
			sendUndo(undo);

			m_materials[textureId]->m_textureData = nullptr;
			m_materials[textureId]->m_filename = "";
			m_materials[textureId]->m_type = Material::MATTYPE_BLANK;
			invalidateTextures();
		}
	}
}

bool Model::setTextureSClamp(unsigned textureId,bool clamp)
{
	if(textureId<m_materials.size())
	{
		MU_SetMaterialClamp *undo = new MU_SetMaterialClamp();
		undo->setMaterialClamp(textureId,true,clamp,
				m_materials[textureId]->m_sClamp);
		sendUndo(undo);

		m_materials[textureId]->m_sClamp = clamp;
		return true;
	}
	return false;
}

bool Model::setTextureTClamp(unsigned textureId,bool clamp)
{
	if(textureId<m_materials.size())
	{
		MU_SetMaterialClamp *undo = new MU_SetMaterialClamp();
		undo->setMaterialClamp(textureId,false,clamp,
				m_materials[textureId]->m_tClamp);
		sendUndo(undo);

		m_materials[textureId]->m_tClamp = clamp;
		return true;
	}
	return false;
}

void Model::noTexture(unsigned id)
{
	LOG_PROFILE();

	for(unsigned t = 0; t<m_groups.size(); t++)
	{
		if((unsigned)m_groups[t]->m_materialIndex==id)
		{
			m_groups[t]->m_materialIndex = -1;
		}
	}
}

#endif // MM3D_EDIT

int Model::getGroupTextureId(unsigned groupNumber)const
{
	if(groupNumber>=0&&groupNumber<m_groups.size())
	{
		return m_groups[groupNumber]->m_materialIndex;
	}
	else
	{
		return -1;
	}
}

bool Model::getTextureCoords(unsigned triangleNumber, unsigned vertexIndex, float &s, float &t)const
{
	if(triangleNumber<m_triangles.size()&&vertexIndex<3)
	{
		s = m_triangles[triangleNumber]->m_s[vertexIndex];
		t = m_triangles[triangleNumber]->m_t[vertexIndex];
		return true;
	}
	else
	{
		return false;
	}
}

Texture *Model::getTextureData(unsigned textureId)
{
	if(textureId>=0&&textureId<m_materials.size())
	{
		return m_materials[textureId]->m_textureData;
	}
	else
	{
		return nullptr;
	}
}

bool Model::getTextureAmbient(unsigned textureId, float *ambient)const
{
	if(ambient&&textureId<m_materials.size())
	{
		for(int t = 0; t<4; t++)
		{
			ambient[t] = m_materials[textureId]->m_ambient[t];
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Model::getTextureDiffuse(unsigned textureId, float *diffuse)const
{
	if(diffuse&&textureId<m_materials.size())
	{
		for(int t = 0; t<4; t++)
		{
			diffuse[t] = m_materials[textureId]->m_diffuse[t];
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Model::getTextureSpecular(unsigned textureId, float *specular)const
{
	if(specular&&textureId<m_materials.size())
	{
		for(int t = 0; t<4; t++)
		{
			specular[t] = m_materials[textureId]->m_specular[t];
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Model::getTextureEmissive(unsigned textureId, float *emissive)const
{
	if(emissive&&textureId<m_materials.size())
	{
		for(int t = 0; t<4; t++)
		{
			emissive[t] = m_materials[textureId]->m_emissive[t];
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Model::getTextureShininess(unsigned textureId, float &shininess)const
{
	if(textureId<m_materials.size())
	{
		shininess = m_materials[textureId]->m_shininess;
		return true;
	}
	else
	{
		return false;
	}
}

bool Model::getTextureSClamp(unsigned textureId)const
{
	if(textureId<m_materials.size())
	{
		return m_materials[textureId]->m_sClamp;
	}
	//https://github.com/zturtleman/mm3d/issues/103
	//return false; //2019
	return true;
}

bool Model::getTextureTClamp(unsigned textureId)const
{
	if(textureId<m_materials.size())
	{
		return m_materials[textureId]->m_tClamp;
	}

	//https://github.com/zturtleman/mm3d/issues/103
	//return false; //2019
	return true; 
}


const char *Model::getTextureName(unsigned textureId)const
{
	if(textureId>=0&&textureId<m_materials.size())
	{
		return m_materials[textureId]->m_name.c_str();
	}
	else
	{
		return nullptr;
	}
}

const char *Model::getTextureFilename(unsigned textureId)const
{
	if(textureId>=0&&textureId<m_materials.size())
	{
		return m_materials[textureId]->m_filename.c_str();
	}
	else
	{
		return nullptr;
	}
}

/*REMOVE ME (Moved to header for time being.)
int Model::getMaterialColor(unsigned materialIndex, unsigned c, unsigned v)const
{
	if(materialIndex<m_materials.size()&&c<4&&v<4)
	{
		return m_materials[materialIndex]->m_color[v][c];
	}
	else
	{
		return 0;
	}
}*/

Model::Material::MaterialTypeE Model::getMaterialType(unsigned materialIndex)const
{
	if(materialIndex<m_materials.size())
	{
		return m_materials[materialIndex]->m_type;
	}
	else
	{
		return Material::MATTYPE_BLANK;
	}
}

int Model::getMaterialByName(const char *const materialName,bool ignoreCase)const
{
	int (*compare)(const char *, const char *);
	compare = ignoreCase ? PORT_strcasecmp : strcmp;

	int matNumber = -1;

	for(unsigned m = 0; m<m_materials.size(); m++)
	{
		if(compare(materialName,m_materials[m]->m_name.c_str())==0)
		{
			matNumber = m;
			break;
		}
	}

	return matNumber;
}

DrawingContext *Model::getDrawingContext(ContextT context)
{
	DrawingContextList::iterator it;
	for(it = m_drawingContexts.begin(); it!=m_drawingContexts.end(); it++)
	{
		if((*it)->m_context==context)
		{
			return (*it);
		}
	}

	DrawingContext *drawContext = new DrawingContext;
	drawContext->m_context = context;
	drawContext->m_valid	= false;
	m_drawingContexts.push_back(drawContext);

	return drawContext;
}

void Model::invalidateTextures()
{
	DrawingContextList::iterator it;
	for(it = m_drawingContexts.begin(); it!=m_drawingContexts.end(); it++)
	{
		(*it)->m_valid = false;
	}

	m_validBspTree = false;
}

void Model::deleteGlTextures(ContextT context)
{
	DrawingContext *drawContext = getDrawingContext(context);
	for(FileTextureMap::const_iterator it = drawContext->m_fileTextures.begin();
			it!=drawContext->m_fileTextures.end(); ++it)
	{
		int texId = it->second;
		if(texId>0)
		{
			glDeleteTextures(1,(GLuint *)&texId);
			s_glTextures--;
			log_debug("GL textures: %d\n",s_glTextures);
		}
	}
	drawContext->m_fileTextures.clear();
	drawContext->m_matTextures.clear();
	drawContext->m_valid = false;
}

void Model::removeContext(ContextT context)
{
	DrawingContext *drawContext = nullptr;
	DrawingContextList::iterator it;
	for(it = m_drawingContexts.begin(); drawContext==nullptr&&it!=m_drawingContexts.end(); it++)
	{
		if((*it)->m_context==context)
		{
			drawContext = *it;
			deleteGlTextures(context);
			break;
		}
	}

	if(drawContext)
	{
		m_drawingContexts.erase(it);
		delete drawContext;
	}
}
