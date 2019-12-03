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

#include "cmdmgr.h"
#include "log.h"

CommandManager *CommandManager::s_instance = nullptr;

CommandManager::CommandManager()
{}
CommandManager::~CommandManager()
{
	log_debug("CommandManager releasing %d commands\n",m_commands.size());
	
	for(auto*ea:m_commands) ea->release();
}

CommandManager *CommandManager::getInstance()
{
	if(s_instance==nullptr)
	{
		s_instance = new CommandManager();
	}

	return s_instance;
}

void CommandManager::release()
{
	if(s_instance!=nullptr)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}

static struct SeparatorCommand : Command
{
	SeparatorCommand():Command(0){}

	virtual void release(){}

	virtual const char *getName(int){ return ""; };

	virtual bool activated(int,Model*){ return false; };

}cmdgr_sep;

void CommandManager::addCommand(Command *cmd, bool separate)
{
	//toolbox.cc doesn't log this. (0) is no longer adequate.
	//log_debug("registering command '%s'\n",cmd->getName(0));
	//m_commands.push_back(cmd);

	//NOTE: Two-levels of submenus is not implemented. If it is
	//deemed necessary, I think the best way is to add a method
	//that adds one more submenu, but can't be shared and comes
	//into play only if getPath is nonempty.
	if(auto p=cmd->getPath())
	for(auto it=m_commands.rbegin();it!=m_commands.rend();it++)	
	if(auto q=(*it)->getPath())
	{
		if(q!=p&&strcmp(p,q)) continue;

		(const void*&)cmd->m_path = q; //SIMPLIFY THINGS

		//HACK: Enable limited separators in submenus for
		//stdcmds.cc macros.
		if(m_commands.back()->isSeparator()) 
		{
			it--; assert(it==m_commands.rbegin());
		}

		auto sep = m_commands.insert(it.base(),cmd);

		//NOTE: Plugin commands should separate a first command.
		if(separate) m_commands.insert(sep,&cmdgr_sep);

		return;
	}
	m_commands.push_back(cmd); 
}
void CommandManager::addSeparator()
{
	m_commands.push_back(&cmdgr_sep); 
}

Command *CommandManager::getFirstCommand()
{
	m_commandIt = m_commands.begin();

	if(m_commandIt!=m_commands.end())
	{
		return *m_commandIt;
	}
	else
	{
		return nullptr;
	}
}

Command *CommandManager::getNextCommand()
{
	if(m_commandIt!=m_commands.end())
	{
		m_commandIt++;
	}

	if(m_commandIt!=m_commands.end())
	{
		return *m_commandIt;
	}
	else
	{
		return nullptr;
	}
}

