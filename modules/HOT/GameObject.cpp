#include "GameObject.h"

#include <scene/main/window.h>

void GameObject::_bind_methods() {
	ClassDB::bind_method(D_METHOD("connectToSignal", "signalName", "callable"), &GameObject::connectToSignal);
	ClassDB::bind_method(D_METHOD("disconnectFromSignal", "signalName", "callable"), &GameObject::disconnectFromSignal);
	ClassDB::bind_method(D_METHOD("hasSignal", "signalName"), &GameObject::hasSignal);
	ClassDB::bind_method(D_METHOD("injectEmitSignal", "signalName", "parameters"), &GameObject::injectEmitSignal);

	ClassDB::bind_method(D_METHOD("getChildNodeWithMethod", "method"), &GameObject::getChildNodeWithMethod);
	ClassDB::bind_method(D_METHOD("getChildNodeWithSignal", "signalName"), &GameObject::getChildNodeWithSignal);
	ClassDB::bind_method(D_METHOD("getChildNodesWithMethod", "method", "fillArray"), &GameObject::getChildNodesWithMethod);
	ClassDB::bind_method(D_METHOD("getChildNodeInGroup", "groupName"), &GameObject::getChildNodeInGroup);

	ClassDB::bind_method(D_METHOD("setInheritModifierFrom", "parentGameObject"), &GameObject::setInheritModifierFrom);
	ClassDB::bind_method(D_METHOD("getInheritModifierFrom"), &GameObject::getInheritModifierFrom);
	ClassDB::bind_method(D_METHOD("triggerModifierUpdated", "modifierType"), &GameObject::triggerModifierUpdated);
	ClassDB::bind_method(D_METHOD("calculateModifiedValue", "modifierType", "baseValue", "categories"), &GameObject::calculateModifiedValue);

	ClassDB::bind_method(D_METHOD("add_effect", "effectScene", "externalSource"), &GameObject::add_effect);
	ClassDB::bind_method(D_METHOD("find_effect", "effectID"), &GameObject::find_effect);

	ClassDB::bind_method(D_METHOD("get_rootSourceGameObject"), &GameObject::get_rootSourceGameObject);
	ClassDB::bind_method(D_METHOD("set_sourceGameObject", "source"), &GameObject::set_sourceGameObject);

	ClassDB::bind_method(D_METHOD("set_spawn_origin", "spawn_origin_node"), &GameObject::set_spawn_origin);
	ClassDB::bind_method(D_METHOD("get_spawn_origin"), &GameObject::get_spawn_origin);

	ADD_SIGNAL(MethodInfo("ModifierUpdated", PropertyInfo(Variant::STRING_NAME, "type")));
	ADD_SIGNAL(MethodInfo("Removed", PropertyInfo(Variant::OBJECT, "type", PROPERTY_HINT_RESOURCE_TYPE, "Node")));
}

void GameObject::_notification(int p_notification) {
	switch(p_notification) {
		case NOTIFICATION_READY:
			_ready();
			break;
		case NOTIFICATION_EXIT_TREE:
			_exit_tree();
			break;
	}
}

void GameObject::_ready() {
	connect("child_entered_tree", callable_mp(this, &GameObject::_child_entered_tree));
}

void GameObject::_exit_tree() {
	disconnect("child_entered_tree", callable_mp(this, &GameObject::_child_entered_tree));
	emit_signal("Removed", this);
}

void GameObject::_child_entered_tree(Node* childNode) {
	// we check all the signal connections, so that we can connect signals
	// even when the child that has the signal was added after the one that
	// wants to connect to it!
	for(auto connectedSignal : _connectedSignals) {
		if(!connectedSignal.CallableConnection.is_valid())
			continue;
		if(childNode->has_signal(connectedSignal.SignalName))
			childNode->connect(connectedSignal.SignalName, connectedSignal.CallableConnection);
	}
}

void fillArrayWithChildrenOfNode(Node* node, std::vector<Node*>& array) {
	for(int i=0; i < node->get_child_count(); ++i)
		array.push_back(node->get_child(i));
}

void GameObject::populateTempNodesWithAllChildren() {
	_tempNodeArray.clear();
	fillArrayWithChildrenOfNode(this, _tempNodeArray);
	int currentIndex = 0;
	while(currentIndex < _tempNodeArray.size()) {
		if(_tempNodeArray[currentIndex]->is_queued_for_deletion())
		{
			_tempNodeArray.erase(_tempNodeArray.begin() + currentIndex);
			continue;
		}
		fillArrayWithChildrenOfNode(_tempNodeArray[currentIndex], _tempNodeArray);
		currentIndex += 1;
	}
}

void GameObject::connectToSignal(StringName signalName, Callable callable) {
	_connectedSignals.push_back({
			signalName,
			callable
	});
	populateTempNodesWithAllChildren();
	for(auto child : _tempNodeArray) {
		if(child->has_signal(signalName))
			child->connect(signalName, callable);
	}
}

void GameObject::disconnectFromSignal(StringName signalName, Callable callable) {
	auto connectedSignalsIter = _connectedSignals.begin();
	while (connectedSignalsIter != _connectedSignals.end()) {
		if(connectedSignalsIter->SignalName == signalName && connectedSignalsIter->CallableConnection == callable)
			connectedSignalsIter = _connectedSignals.erase(connectedSignalsIter);
		else
			++connectedSignalsIter;
	}
	populateTempNodesWithAllChildren();
	for(auto child : _tempNodeArray) {
		if(child->has_signal(signalName))
			child->disconnect(signalName, callable);
	}
}

bool GameObject::hasSignal(StringName signalName) {
	populateTempNodesWithAllChildren();
	for(auto child : _tempNodeArray) {
		if(child->has_signal(signalName))
			return true;
	}
	return false;
}

void GameObject::injectEmitSignal(StringName signalName, Array parameters) {
	for(auto signalConnection : _connectedSignals) {
		if(signalConnection.SignalName == signalName && signalConnection.CallableConnection.is_valid())
			signalConnection.CallableConnection.callv(parameters);
	}
}

Node* GameObject::getChildNodeWithMethod(StringName methodName) {
	_tempNodeArray.clear();
	fillArrayWithChildrenOfNode(this, _tempNodeArray);
	int currentIndex = 0;
	while(currentIndex < _tempNodeArray.size()) {
		if(_tempNodeArray[currentIndex]->is_queued_for_deletion())
		{
			_tempNodeArray.erase(_tempNodeArray.begin() + currentIndex);
			continue;
		}
		if(_tempNodeArray[currentIndex]->has_method(methodName))
			return _tempNodeArray[currentIndex];
		fillArrayWithChildrenOfNode(_tempNodeArray[currentIndex], _tempNodeArray);
		currentIndex += 1;
	}
	return nullptr;
}

Node* GameObject::getChildNodeWithSignal(StringName signalName) {
	_tempNodeArray.clear();
	fillArrayWithChildrenOfNode(this, _tempNodeArray);
	int currentIndex = 0;
	while(currentIndex < _tempNodeArray.size()) {
		if(_tempNodeArray[currentIndex]->is_queued_for_deletion())
		{
			_tempNodeArray.erase(_tempNodeArray.begin() + currentIndex);
			continue;
		}
		if(_tempNodeArray[currentIndex]->has_signal(signalName))
			return _tempNodeArray[currentIndex];
		fillArrayWithChildrenOfNode(_tempNodeArray[currentIndex], _tempNodeArray);
		currentIndex += 1;
	}
	return nullptr;
}

void GameObject::getChildNodesWithMethod(StringName methodName, Array fillArray) {
	populateTempNodesWithAllChildren();
	for(auto child : _tempNodeArray) {
		if(child->has_method(methodName))
			fillArray.append(child);
	}
}

Node* GameObject::getChildNodeInGroup(StringName groupName) {
	_tempNodeArray.clear();
	fillArrayWithChildrenOfNode(this, _tempNodeArray);
	int currentIndex = 0;
	while(currentIndex < _tempNodeArray.size()) {
		if(_tempNodeArray[currentIndex]->is_queued_for_deletion())
		{
			_tempNodeArray.erase(_tempNodeArray.begin() + currentIndex);
			continue;
		}
		if(_tempNodeArray[currentIndex]->is_in_group(groupName))
			return _tempNodeArray[currentIndex];
		fillArrayWithChildrenOfNode(_tempNodeArray[currentIndex], _tempNodeArray);
		currentIndex += 1;
	}
	return nullptr;
}

void GameObject::setInheritModifierFrom(GameObject *otherGameObject) {
	if(_inheritModifierFrom != nullptr && !_inheritModifierFrom->is_queued_for_deletion())
		_inheritModifierFrom->disconnect("ModifierUpdated", callable_mp(this, &GameObject::triggerModifierUpdated));
	_inheritModifierFrom = otherGameObject;
	if(_inheritModifierFrom != nullptr && !_inheritModifierFrom->is_queued_for_deletion())
		_inheritModifierFrom->connect("ModifierUpdated", callable_mp(this, &GameObject::triggerModifierUpdated));
	triggerModifierUpdated("ALL");
}

GameObject *GameObject::getInheritModifierFrom() {
	return _inheritModifierFrom;
}

void GameObject::triggerModifierUpdated(StringName modifierType) {
	emit_signal("ModifierUpdated", modifierType);
}

Variant GameObject::calculateModifiedValue(StringName modifierType, Variant baseValue, TypedArray<String> categories) {
	int baseValueInt = baseValue;
	float baseValueFloat = baseValue;
	float multiplier = 1;
	String addModFunc("mod_"); addModFunc += modifierType; addModFunc += "_additive";
	String mulModFunc("mod_"); mulModFunc += modifierType; mulModFunc += "_percentage";
	_tempNodeArray.clear();
	fillArrayWithChildrenOfNode(this, _tempNodeArray);
	if(_inheritModifierFrom != nullptr && !_inheritModifierFrom->is_queued_for_deletion())
		fillArrayWithChildrenOfNode(_inheritModifierFrom, _tempNodeArray);
	for(auto childNode : _tempNodeArray) {
		if(childNode->is_queued_for_deletion())
			continue ;
		if(childNode->has_method("mod_allow_categories") && !(bool)childNode->call("mod_allow_categories", categories))
			continue ;
		if(childNode->has_method(addModFunc)){
			Variant addModRes = childNode->call(addModFunc);
			baseValueInt += (int)addModRes;
			baseValueFloat += (float)addModRes;
		}
		if(childNode->has_method(mulModFunc))
			multiplier += (float)childNode->call(mulModFunc);
	}

	if(baseValue.get_type() == Variant::FLOAT) {
		if(multiplier < 0) return 0.0f;
		return baseValueFloat * multiplier;
	}
	if(multiplier < 0) return 0;
	return (int)(baseValueInt * multiplier);
}

Node* GameObject::add_effect(Node *effectScene, GameObject *externalSource) {
	String effectID = effectScene->call("get_effectID");
	Node* GlobalQuestPool =
			get_tree()->get_root()->get_node(NodePath("Global/QuestPool"));
	GlobalQuestPool->call("notify_effect_applied", effectID);
	_tempNodeArray.clear();
	fillArrayWithChildrenOfNode(this, _tempNodeArray);
	for(auto child : _tempNodeArray) {
		if(child->is_queued_for_deletion())
			continue ;
		if(child->has_method("get_effectID") && effectID == (String)child->call("get_effectID")) {
			child->call("add_additional_effect", effectScene);
			return child;
		}
	}
	auto effectInstance = effectScene->duplicate(
			Node::DUPLICATE_GROUPS | Node::DUPLICATE_SCRIPTS | Node::DUPLICATE_SIGNALS);
	effectInstance->call("set_externalSource", externalSource);
	add_child(effectInstance);
	return effectInstance;
}

Node* GameObject::find_effect(String effectID) {
	_tempNodeArray.clear();
	fillArrayWithChildrenOfNode(this, _tempNodeArray);
	for(auto child : _tempNodeArray) {
		if(child->is_queued_for_deletion())
			continue ;
		if(child->has_method("get_effectID") && effectID == (String)child->call("get_effectID"))
			return child;
	}
	return nullptr;
}

GameObject* GameObject::get_rootSourceGameObject() {
	auto sourceIter = _sourceTree.rbegin();
	while(sourceIter != _sourceTree.rend()) {
		auto sourceObj = sourceIter->get_validated_object();
		if(sourceObj != nullptr) {
			return dynamic_cast<GameObject *>(sourceObj);
		}
		sourceIter++;
	}
	return this;
}

void GameObject::set_sourceGameObject(GameObject *source) {
	_sourceTree.clear();
	if(source == nullptr)
		return;
	_sourceTree.push_back(source);
	if(!source->_sourceTree.empty())
		_sourceTree.insert(_sourceTree.end(), source->_sourceTree.begin(), source->_sourceTree.end());
}

void GameObject::set_spawn_origin(Node *spawnOrigin) {
	_spawn_origin = spawnOrigin;
}

Node* GameObject::get_spawn_origin() {
	if(_spawn_origin.is_null())
		return nullptr;
	return (Node*)_spawn_origin.get_validated_object();
}

