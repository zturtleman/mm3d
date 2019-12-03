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

#include "filtermgr.h"
#include "modelfilter.h"
#include "log.h"

FilterManager *FilterManager::s_instance = nullptr;

FilterManager::~FilterManager()
{
	log_debug("FilterManager releasing %d filters\n",m_filters.size());
	for(auto*ea:m_filters) ea->release();
}

FilterManager *FilterManager::getInstance()
{
	if(!s_instance)
	{
		s_instance = new FilterManager();
	}

	return s_instance;
}

void FilterManager::release()
{
	if(s_instance!=nullptr)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}

void FilterManager::registerFilter(ModelFilter *filter)
{
	_read.clear(); _write.clear(); _export.clear();
	
	m_filters.push_back(filter); assert(filter);

	filter->setFactory(&m_factory); //???
}
static const char *filtermgr_all_types //DUPLICATES texmgr.cc
(std::vector<ModelFilter*> &f, std::string &g, const char*(ModelFilter::*mf)())
{
	if(!g.empty()) return g.c_str(); //???

	//log_debug("have %d filters\n",f.size());

	for(auto ea:f)
	{
		const char *t = (ea->*mf)();

		if(!t||!*t){ assert(t); continue; }

		size_t dup = g.size();

		//NOT REMOVING NONEXISTANT DUPLICATES.
		g.append(t); g.push_back(' ');
	}

	if(!g.empty()) g.pop_back(); return g.c_str();
}
const char *FilterManager::getAllReadTypes()
{
	return filtermgr_all_types(m_filters,_read,&ModelFilter::getReadTypes);
}
const char *FilterManager::getAllWriteTypes(bool exportModel)
{
	if(!exportModel)
	return filtermgr_all_types(m_filters,_write,&ModelFilter::getWriteTypes);
	return filtermgr_all_types(m_filters,_export,&ModelFilter::getExportTypes);
}

Model::ModelErrorE FilterManager::readFile(Model *model, const char *filename)
{
	//NEW: The individual filters are reimplementing this??
	if(!model||!filename||!*filename) 
	{
		log_error("no filename supplied for model filter");
		return Model::ERROR_BAD_ARGUMENT;
	}

	//OPTIMIZING
	const char *ext = strrchr(filename,'.'); //NEW
	if(!ext) ext = filename;

	Model::ModelErrorE rval = Model::ERROR_UNKNOWN_TYPE;
	
	for(auto*ea:m_filters) 
	{  
		//if(ea&&ea->isSupported(filename)) //???
		if(ea->canRead(ext))
		{
			//if(ea->canRead()) //???
			{
				model->setUndoEnabled(false);
				model->forceAddOrDelete(true);

				rval = ea->readFile(model,filename);

				model->forceAddOrDelete(false);
				model->setUndoEnabled(true);
				model->clearUndo();
				m_factory.closeAll();

				return rval;
			}
			//else
			{
			//	rval = Model::ERROR_UNSUPPORTED_OPERATION;
			}
		}
	}

	return rval;
}

Model::ModelErrorE FilterManager::writeFile(Model *model, const char *filename, bool exportModel, FilterManager::WriteOptionsE wo)
{	
	//NEW: The individual filters are reimplementing this??
	if(!model||!filename||!*filename) 
	{
		log_error("no filename supplied for model filter");
		return Model::ERROR_BAD_ARGUMENT;
	}

	bool canWrite = true;
	bool tryExport = false;
	for(auto*ea:m_filters) 
	{
		if(ea&&ea->isSupported(filename))
		if(exportModel&&ea->canExport()||ea->canWrite())
		{
			ModelFilter::PromptF *f = ea->getOptionsPrompt();
			ModelFilter::Options *o = ea->getDefaultOptions();		
			if(wo==WO_ModelNoPrompt) o->setOptionsFromModel(model);
			
			bool doWrite = true;
			if(f!=nullptr&&o!=nullptr&&wo!=WO_ModelNoPrompt)
			{
				doWrite = f(model,o);
			}

			Model::ModelErrorE err = Model::ERROR_NONE;

			if(doWrite)
			{
				err = ea->writeFile(model,filename,*o);
				m_factory.closeAll();
			}
			else err = Model::ERROR_CANCEL;

			o->release(); return err;
		}
		else if(!exportModel&&ea->canExport())
		{
			tryExport = true;
		}
		else canWrite = false;
	}

	if(tryExport)
	{
		return Model::ERROR_EXPORT_ONLY;
	}
	if(canWrite)
	{
		return Model::ERROR_UNKNOWN_TYPE;
	}
	else
	{
		return Model::ERROR_UNSUPPORTED_OPERATION;
	}
}