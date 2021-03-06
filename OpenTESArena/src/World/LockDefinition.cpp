#include "LockDefinition.h"

#include "components/debug/Debug.h"

void LockDefinition::LeveledLockDef::init(int lockLevel)
{
	this->lockLevel = lockLevel;
}

void LockDefinition::KeyLockDef::init()
{
	// Do nothing.
}

void LockDefinition::init(WEInt x, int y, SNInt z, Type type)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->type = type;
}

LockDefinition LockDefinition::makeLeveledLock(WEInt x, int y, SNInt z, int lockLevel)
{
	LockDefinition lockDef;
	lockDef.init(x, y, z, Type::LeveledLock);
	lockDef.leveledLock.init(lockLevel);
	return lockDef;
}

LockDefinition LockDefinition::makeKeyLock(WEInt x, int y, SNInt z)
{
	LockDefinition lockDef;
	lockDef.init(x, y, z, Type::KeyLock);
	lockDef.keyLock.init();
	return lockDef;
}

WEInt LockDefinition::getX() const
{
	return this->x;
}

int LockDefinition::getY() const
{
	return this->y;
}

SNInt LockDefinition::getZ() const
{
	return this->z;
}

LockDefinition::Type LockDefinition::getType() const
{
	return this->type;
}

const LockDefinition::LeveledLockDef &LockDefinition::getLeveledLockDef() const
{
	DebugAssert(this->type == Type::LeveledLock);
	return this->leveledLock;
}

const LockDefinition::KeyLockDef &LockDefinition::getKeyLockDef() const
{
	DebugAssert(this->type == Type::KeyLock);
	return this->keyLock;
}
