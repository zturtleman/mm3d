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

#include "undomgr.h"

#include "log.h"
#include "undo.h"

UndoManager::UndoManager()
	: m_currentUndo(nullptr),
	  m_currentList(nullptr),
	  m_listCombine(true),
	  m_sizeLimit(0),
	  m_countLimit(0),
	  m_saveLevel(0)
{
}

UndoManager::~UndoManager()
{
	clear();
}

void UndoManager::setSaved()
{
	m_saveLevel = 0;
}

bool UndoManager::isSaved()const
{
	return (m_saveLevel==0)? true : false;
}

void UndoManager::clear()
{
	operationComplete("Doomed operation"); //???
	
	for(UndoList*ea:m_atomic)
	{
		for(Undo*ea2:*ea)
		{
			ea2->undoRelease(); //NOTE: Not redoRelease?
			
			ea2->release();
		}
		delete ea;
	}
	m_atomic.clear();

	clearRedo();
}
void UndoManager::clearRedo()
{
	for(UndoList*ea:m_atomicRedo)
	{
		for(Undo*ea2:*ea)
		{
			ea2->redoRelease(); //NOTE: Not undoRelease?
			
			ea2->release();
		}
		delete ea;
	}
	m_atomicRedo.clear();
}
void UndoManager::addUndo(Undo *u, bool listCombine)
{
	clearRedo();

	if(!listCombine)
	{
		m_listCombine = false;
	}

	if(m_currentUndo)
	{
		// Try to combine these undo items
		if(combineWithList(u))
		{
			// Combined, release new one
			u->release();
		}
		else
		{
			// Push the current undo onto the current list and make this undo current
			pushUndoToList(m_currentUndo);
			m_currentUndo = u;
		}
	}
	else
	{
		// No undo yet, this is our first
		m_currentUndo = u;
	}
}

void UndoManager::operationComplete(const char *opname)
{
	// if we have anything to undo
	if(m_currentUndo)
	{
		//Ensure this for Model::appendUndo.
		if(!*opname||!*opname)
		{
			//For some reason operationComplete() is 
			//called by undo() and redo(). I think if
			//it gets here, perhaps appendUndo is best.
			//assert(*opname);
			opname = "<Unnamed Undo>";
		}

		log_debug("operation complete: %s\n",opname);
		m_saveLevel++;
		pushUndoToList(m_currentUndo);
		m_currentList->setOpName(opname);
		m_atomic.push_back(m_currentList);
	}
	else
	{
		log_debug("nothing to undo\n");
	}
	m_currentList = nullptr;
	m_currentUndo = nullptr;
	m_listCombine = true;

	checkSize();

	showStatistics();
}

UndoList *UndoManager::undo()
{
	LOG_PROFILE();

	operationComplete();

	if(!m_atomic.empty())
	{
		m_atomicRedo.push_back(m_atomic.back());
		m_atomic.pop_back();

		showStatistics();

		log_debug("Undo: %s\n",m_atomicRedo.back()->getOpName());

		m_saveLevel--;

		return m_atomicRedo.back();
	}
	else return nullptr;
}

UndoList *UndoManager::redo()
{
	LOG_PROFILE();

	operationComplete();

	if(!m_atomicRedo.empty())
	{
		m_atomic.push_back(m_atomicRedo.back());
		m_atomicRedo.pop_back();

		showStatistics();

		log_debug("Redo: %s\n",m_atomic.back()->getOpName());

		m_saveLevel++;

		return m_atomic.back();
	}
	else return nullptr;
}

const char *UndoManager::getUndoOpName()const
{
	if(m_currentUndo) //???
	{
		return "";
	}
	else if(!m_atomic.empty())
	{
		return m_atomic.back()->getOpName();
	}
	else return "";
}

const char *UndoManager::getRedoOpName()const
{
	if(!m_atomicRedo.empty())
	{
		return m_atomicRedo.back()->getOpName();
	}
	else return "";
}

UndoList *UndoManager::undoCurrent()
{
	LOG_PROFILE();

	if(m_currentUndo)
	{
		operationComplete("Partial operation");

		if(!m_atomic.empty())
		{
			m_atomicRedo.push_back(m_atomic.back());
			m_atomic.pop_back();

			log_debug("Undo: %s\n",m_atomicRedo.back()->getOpName());
			return m_atomicRedo.back();
		}
	}
	return nullptr;
}

bool UndoManager::combineWithList(Undo *u)
{
	if(m_currentUndo->combine(u))
	{
		return true;
	}
	else if(m_listCombine)
	{
		if(m_currentList)
		{
			UndoList::iterator it;
			for(it = m_currentList->begin(); it!=m_currentList->end(); it++)
			{
				if((*it)->combine(u))
				{
					return true;
				}
			}
		}
	}
	return false;
}

void UndoManager::pushUndoToList(Undo *u)
{
	if(!m_currentList)
	{
		m_currentList = new UndoList;
	}
	m_currentList->push_back(u);
}

void UndoManager::showStatistics()const
{
	int undoItems = 0;
	int redoItems = 0;
	unsigned undoSize = 0;
	unsigned redoSize = 0;

	AtomicList::const_iterator it;
	UndoList::const_iterator uit;

	log_debug("Undo:\n");
	for(it = m_atomic.begin(); it!=m_atomic.end(); it++)
	{
		log_debug("  %s\n",(*it)->getOpName());
		undoItems += (*it)->size();
		for(uit = (*it)->begin(); uit!=(*it)->end(); uit++)
		{
			undoSize += (*uit)->size();
		}
	}
	log_debug("\n");

	log_debug("Redo:\n");
	for(it = m_atomicRedo.begin(); it!=m_atomicRedo.end(); it++)
	{
		log_debug("  %s\n",(*it)->getOpName());
		redoItems += (*it)->size();
		for(uit = (*it)->begin(); uit!=(*it)->end(); uit++)
		{
			redoSize += (*uit)->size();
		}
	}
	log_debug("\n");

	log_debug("--------------- Undo statistics ---------------\n");
	log_debug(" undo:  %7d size,%5d items,%5d lists\n",undoSize,undoItems,m_atomic.size());
	log_debug(" redo:  %7d size,%5d items,%5d lists\n",redoSize,redoItems,m_atomicRedo.size());
	log_debug(" total: %7d size,%5d items,%5d lists\n",undoSize+redoSize,undoItems+redoItems,m_atomic.size()+m_atomicRedo.size());
	log_debug("-----------------------------------------------\n");
}

size_t UndoManager::getSize() //NEW
{
	size_t sum = 0;
	for(auto*ea:m_atomic) for(auto*ea2:*ea)
	sum+=ea2->size(); return sum;
}
void UndoManager::checkSize()
{	
	size_t count1 = 0, count2 = 0;

	if(m_sizeLimit) //2019: Ignoring MAX_UNDO_LIST_SIZE
	{
		size_t size = getSize();

		if(size>m_sizeLimit)
		{
			log_debug("Undo list size is %d,freeing a list\n",size);

			auto it = m_atomic.begin();
			for(auto itt=m_atomic.end();it<itt;it++)
			{
				size_t sum = 0;
				for(Undo*ea:**it) sum+=ea->size();

				size-=sum;
				
				if(size>m_sizeLimit) 
				{
					count1++;
				}
				else break;
			}
		}
	}

	if(m_countLimit)
	if(m_atomic.size()>m_countLimit)
	{
		count2 = m_atomic.size()-m_countLimit;
	}

	size_t count = std::max(count1,count2);

	auto itt = m_atomic.begin()+count;
	for(auto it=itt-count;it<itt;it++)
	{
		for(Undo*ea:**it)
		{
			ea->undoRelease(); ea->release();
		}
		delete *it;
	}
	m_atomic.erase(itt-count,itt);
}
