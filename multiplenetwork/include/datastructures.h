/**
 * datastructures.h
 *
 * Author: Matteo Magnani <matteo.magnani@it.uu.se>
 * Version: 1.0
 *
 * This file defines the basic data structures of the library. It includes:
 * 
 * 1) Basic components of a MLNetwork (layer, node, edge, actor).
 *    An actor represents a global identity, and multiple nodes
 *    (organized into multiple layers) can correspond to the same actor.
 *    An example is an individual (actor) with multiple accounts (nodes)
 *    on different online social networks (layers).
 * 2) Smart pointers to the basic MLNetwork components.
 *    Only one instance of each entity (e.g., node) is kept in memory,
 *    and can be accessed through multiple indexes (e.g., by ID, by name)
 *    storing pointers to the entities. Functions accessing the MLNetwork's
 *    components return pointers as well, so that the objects are never
 *    duplicated after their creation.
 * 3) Iterators, used for efficiency reasons: functions potentially returning
 *    many pointers to entities (e.g., "return all neighbors of a node") do not
 *    directly return all the pointers, but only an iterator over them.
 *    Iterators are not currently thread safe: a modification in a MLNetwork
 *    while an iterator is active does not invalidate the iterator and can
 *    produce inconsistent results when the iterator methods are called.
 * 4) An AttributeStore class, to associate attributes to different objects
 *    (actors, nodes...) in the MLNetwork.
 * 5) MLNetwork, the main class defined in this file.
 *
 * All the definitions are in the "mlnet" namespace
 */

#ifndef MLNET_DATASTRUCTURES_H_
#define MLNET_DATASTRUCTURES_H_

#include <string>
#include <map>
#include "exceptions.h"
#include "random.h"
#include <cmath>
#include <set>
#include <vector>
#include <memory>
#include <cstdlib>

namespace mlnet {

class MLNetwork;
typedef std::shared_ptr<MLNetwork> MLNetworkSharedPtr;
typedef std::shared_ptr<const MLNetwork> constMLNetworkSharedPtr;

/**********************************************************************/
/** Constants *********************************************************/
/**********************************************************************/

const bool DEFAULT_EDGE_DIRECTIONALITY = false; // undirected edges by default

// Selection mode, for directed edges (e.g., to compute the IN-degree or OUT-degree of a node)
enum edge_mode {INOUT=0, IN=1, OUT=2};

// Supported attribute types
enum attribute_type {STRING_TYPE = 0, NUMERIC_TYPE = 1};

/**********************************************************************/
/** MLNetwork components **********************************************/
/**********************************************************************/

// Identifiers

/** A generic identifier for all objects in a MLNetwork (nodes, edges, ...) */
typedef long object_id;
/** The unique identifier of each node inside a MLNetwork */
typedef long node_id;
/** The unique identifier of each edge inside a MLNetwork */
typedef long edge_id;
/** The unique identifier of each layer in a MLNetwork. Every node belongs to exactly one layer */
typedef int  layer_id;
/** Nodes in different layers may correspond to the same "actor",
 * e.g., a person (actor) can have multiple accounts (nodes) on
 * different social media (layers) */
typedef long actor_id;

// objects

/**
 * An actor in a MLNetwork.
 */
class actor {
public:
	actor_id id;
	std::string name;
	/* Constructor */
	actor(actor_id id, const std::string& name);
	~actor();
	/* Comparison operators */
	bool operator==(const actor& a) const;
	bool operator!=(const actor& a) const;
	bool operator<(const actor& a) const;
	bool operator>(const actor& a) const;
	/* Output */
	std::string to_string() const;
};

typedef std::shared_ptr<actor> ActorSharedPtr;

/**
 * A layer in a MLNetwork.
 */
class layer {
public:
	layer_id id;
	std::string name;
	/* Constructor */
	layer(const layer_id& id, const std::string& name);
	~layer();
	/* Comparison operators */
	bool operator==(const layer& l) const;
	bool operator!=(const layer& l) const;
	bool operator<(const layer& l) const;
	bool operator>(const layer& l) const;
	/* Output */
	std::string to_string() const;
};

typedef std::shared_ptr<layer> LayerSharedPtr;

/**
 * A node inside a MLNetwork.
 */
class node {
public:
	node_id id;
	std::string name;
	ActorSharedPtr actor;
	LayerSharedPtr layer;
	/* Constructor */
	node(const node_id& id, const std::string& name, const ActorSharedPtr& actor, const LayerSharedPtr& layer);
	~node();
	/* Comparison operators */
	bool operator==(const node& v) const;
	bool operator!=(const node& v) const;
	bool operator<(const node& v) const;
	bool operator>(const node& v) const;
	/* Output */
	std::string to_string() const;
};

typedef std::shared_ptr<node> NodeSharedPtr;

/**
 * An edge between two nodes in a MLNetwork.
 */
class edge {
public:
	edge_id id;
	NodeSharedPtr v1;
	NodeSharedPtr v2;
	bool directed;
	/* Constructor */
	edge(const edge_id& id, const NodeSharedPtr& v1, const NodeSharedPtr& v2, bool directed);
	~edge();
	/* Comparison operators */
	bool operator==(const edge& e) const;
	bool operator!=(const edge& e) const;
	bool operator<(const edge& e) const;
	bool operator>(const edge& e) const;
	/* Output */
	std::string to_string() const;
};

typedef std::shared_ptr<edge> EdgeSharedPtr;

/**********************************************************************/
/** Iterators *********************************************************/
/**********************************************************************/

const float P = 0.5;
const int MAX_LEVEL = 6;

template <class T>
struct Entry {
	object_id value;
	T obj_ptr;
    std::vector<Entry<T>*> forward; // array of pointers
    std::vector<int> link_length;

    Entry(int level, object_id value, T obj_ptr) {
        forward.resize(level+1);
        link_length.resize(level+1);
        this->value = value;
        this->obj_ptr = obj_ptr;
    }

    ~Entry() {}
};

template <class T>
struct ObjectStore {
    Entry<T> *header;
    int level;
    long num_entries=0;

    ObjectStore() {
        header = new Entry<T>(MAX_LEVEL, 0, NULL);
        level = 0;
    }
    ~ObjectStore() {
        delete header;
    }

	class iterator {
	    typedef std::forward_iterator_tag iterator_category;
		public:
		iterator();
		iterator(Entry<T>* iter);
		T operator*();
		iterator operator++();
		iterator operator++(int);
	    bool operator==(const ObjectStore<T>::iterator& rhs);
	    bool operator!=(const ObjectStore<T>::iterator& rhs);
		private:
		Entry<T>* current;
	};
	ObjectStore<T>::iterator begin() const;
	ObjectStore<T>::iterator end() const;
    long size() const;
    bool contains(object_id) const;
    T get(object_id) const;
    T get_at_index(long) const;
    //T& operator[](object_id);
    void insert(object_id,T);
    void erase(object_id);
    //void print(int);
};



template <class T>
typename ObjectStore<T>::iterator ObjectStore<T>::begin() const {
	return iterator(header->forward[0]);
}

template <class T>
typename ObjectStore<T>::iterator ObjectStore<T>::end() const {
	return iterator(NULL);
}

template <class T>
T ObjectStore<T>::iterator::operator*() {
	return current->obj_ptr;
}

template <class T>
ObjectStore<T>::iterator::iterator(Entry<T>* iter) : current(iter) {
}

template <class T>
typename ObjectStore<T>::iterator ObjectStore<T>::iterator::operator++() { // PREFIX
	current=current->forward[0];
	return *this;
}

template <class T>
typename ObjectStore<T>::iterator ObjectStore<T>::iterator::operator++(int) { // POSTFIX
	ObjectStore<T>::iterator tmp(current);
	current=current->forward[0];
	return tmp;
}

template <class T>
bool ObjectStore<T>::iterator::operator==(const ObjectStore<T>::iterator& rhs) {
	return current == rhs.current;
}

template <class T>
bool ObjectStore<T>::iterator::operator!=(const ObjectStore<T>::iterator& rhs) {
	return current != rhs.current;
}

template <class T>
long ObjectStore<T>::size() const {
	return num_entries;
}

template <class T>
bool ObjectStore<T>::contains(object_id search_value) const {
    const Entry<T> *x = header;
    for (int i = level; i >= 0; i--) {
        while (x->forward[i] != NULL && x->forward[i]->value < search_value) {
            x = x->forward[i];
        }
    }
    x = x->forward[0];
    return x != NULL && x->value == search_value;
}

template <class T>
T ObjectStore<T>::get(object_id search_value) const {
    const Entry<T> *x = header;
    for (int i = level; i >= 0; i--) {
        while (x->forward[i] != NULL && x->forward[i]->value < search_value) {
            x = x->forward[i];
        }
    }
    x = x->forward[0];
    if (x != NULL && x->value == search_value)
    	return x->obj_ptr;
    else return NULL;
}

template <class T>
T ObjectStore<T>::get_at_index(long pos) const {
	if (pos < 0 || pos >= num_entries)
		throw ElementNotFoundException("ObjectStore: out of bounds");
    const Entry<T> *x = header;
    long so_far=0;
    for (int i = level; i >= 0; i--) {
        while (x->forward[i] != NULL && x->link_length[i] + so_far <= pos + 1) {
        	so_far+= x->link_length[i];
            x = x->forward[i];
        }
    }
    return x->obj_ptr;
}

//template <class T>
//void ObjectStore<T>::print(int lev) {
//    const Entry<T> *x = header;
//    while (x!=NULL) {
//    	std::cout << x->value << "(" << x->link_length[lev] << ") ";
//    	for (int i=0; i<x->link_length[lev]-1; i++) std::cout << "     ";
//        x = x->forward[lev];
//    }
//    std::cout << "X\n";
//}

template <class T>
void ObjectStore<T>::insert(object_id value, T obj_ptr) {
    Entry<T> *x = header;
    std::vector<Entry<T>*> update;
    update.resize(MAX_LEVEL+1);
    std::vector<int> skipped_positions_per_level;
    int skipped_positions = 0;
    skipped_positions_per_level.resize(MAX_LEVEL+1,0);


    for (int i = level; i >= 0; i--) {
    	skipped_positions_per_level[i] = skipped_positions;
        while (x->forward[i] != NULL && x->forward[i]->value < value) {
        	skipped_positions_per_level[i] += x->link_length[i];
        	skipped_positions += x->link_length[i];
            x = x->forward[i];
        }
        update[i] = x;
    }
    x = x->forward[0];

    if (x == NULL || x->value != value) {
    	num_entries++;
        int lvl = random_utils::random_level(MAX_LEVEL,P);

        if (lvl > level) {
        	for (int i = level + 1; i <= lvl; i++) {
        		update[i] = header;
        		update[i]->link_length[i] = num_entries;
        	}
        	level = lvl;
        }
        x = new Entry<T>(lvl, value, obj_ptr);
        for (int i = 0; i <= lvl; i++) {
        	int offset = skipped_positions-skipped_positions_per_level[i];

        	x->forward[i] = update[i]->forward[i];
        	if (update[i]->forward[i]==NULL)
        		x->link_length[i] = num_entries - skipped_positions;
        	else {
        		x->link_length[i] = update[i]->link_length[i]-offset;
        	}

        	update[i]->forward[i] = x;
        	update[i]->link_length[i] = offset+1;
        }
        for (int i = lvl+1; i <= level; i++) {
        	update[i]->link_length[i]++;
        }
    }
    else {
    	x->obj_ptr = obj_ptr;
    }
}

template <class T>
void ObjectStore<T>::erase(object_id value) {
	Entry<T> *x = header;
	std::vector<Entry<T>*> update;
	update.resize(MAX_LEVEL+1);

	for (int i = level; i >= 0; i--) {
		while (x->forward[i] != NULL && x->forward[i]->value < value) {
			x = x->forward[i];
		}
		update[i] = x;
	}
	x = x->forward[0];

	if (x== NULL) return;

	if (x->value == value) {
		for (int i = 0; i <= level; i++) {
			if (update[i]->forward[i] != x) {
				update[i]->link_length[i]--;
			}
			else {
				update[i]->forward[i] = x->forward[i];
				update[i]->link_length[i] += x->link_length[i]-1;
			}
		}
		delete x;
		num_entries--;
		while (level > 0 && header->forward[level] == NULL) {
			level--;
		}
	}
}

/**********************************************************************/
/** Attribute handling ************************************************/
/**********************************************************************/

class Attribute {
public:
	/**
	 * Creates a new Attribute.
	 * @param name name of the attribute
	 * @param type type of the attribute (see attribute_type enumeration: STRING_TYPE, NUMERIC_TYPE)
	 */
	Attribute(const std::string& name, attribute_type type);

	/**
	 * @brief Returns the name of the attribute
	 * @return the name of the attribute.
	 **/
	const std::string& name() const;

	/**
	 * @brief Returns the type of the attribute.
	 * @return the type of the attribute.
	 **/
	int type() const;

	/**
	 * @brief Returns a string representation of the type of the attribute.
	 * @return a string representation of the type of the attribute.
	 **/
	std::string type_as_string() const;

private:
	std::string aname;
	attribute_type atype;
};

typedef std::shared_ptr<Attribute> AttributeSharedPtr;

/**
 * This class does not check if objects exist and does not require objects
 * to be explicitly registered into it. Whenever an object that has not been
 * explicitly registered is queried, default attribute values are returned.
 * Therefore, checks about the existence of an object should be performed at
 * the MLNetwork level.
 */
class AttributeStore {
public:
	/**
	 * @brief Returns the number of attributes in this store, of all types.
	 * @return the number of attributes in this store
	 **/
	int numAttributes() const;

	/**
	 * @brief Returns the attributes in this store.
	 * @return a vector containing all the attributes.
	 **/
	const std::vector<AttributeSharedPtr>& attributes() const;

	/**
	 * @brief Returns the n^th attribute in this store.
	 * @param idx the position of the attribute (from 0 to numAttributes()-1).
	 * @return an object of type Attribute, or NULL if idx's attribute does not exist.
	 **/
	AttributeSharedPtr attribute(int idx) const;

	/**
	 * @brief Returns an attribute by name.
	 * @param name The name of the queried attribute.
	 * @return an object of type Attribute, or NULL if an attribute with this name does not exist.
	 **/
	AttributeSharedPtr attribute(const std::string& name) const;

	/**
	 * @brief Enables the association of a value to each object in this store.
	 * @param attribute_name The name of the attribute
	 * @param attribute_type The type of the attribute
	 * STRING_TYPE: c++ "std::string" type
	 * NUMERIC_TYPE: c++ "double" type
	 * @throws DuplicateElementException if an attribute with this name already exists
	 **/
	void add(const std::string& attribute_name, attribute_type type);

	/**
	 * @brief Sets the value of an attribute.
	 * @param object_id the id of the object whose associated value is set
	 * @param attribute_name The name of the attribute
	 * @param value The value to be set
	 * @throws ElementNotFoundException if there is no attribute with this name
	 * @throws OperationNotSupportedException if the attribute type is not STRING_TYPE
	 **/
	void setString(const object_id& oid, const std::string& attribute_name, const std::string& value);

	/**
	 * @brief Sets the value of an attribute.
	 * @param object_id the id of the object whose associated value is set
	 * @param attribute_name The name of the attribute
	 * @param value The value to be set
	 * @throws ElementNotFoundException if there is no attribute with this name
	 * @throws OperationNotSupportedException if the attribute type is not NUMERIC_TYPE
	 **/
	void setNumeric(const object_id& oid, const std::string& attribute_name, double value);

	/**
	 * @brief Gets the value of an attribute.
	 * @param object_id the id of the object whose associated value is retrieved
	 * @param attribute_name The name of the attribute
	 * @return The value associated to the object, or null if the object id has not been registered in this store
	 * @throws ElementNotFoundException if there is no attribute with this name
	 **/
	const std::string& getString(const object_id& oid, const std::string& attribute_name) const;

	/**
	 * @brief Gets the value of an attribute.
	 * @param object_id the id of the object whose associated value is retrieved
	 * @param attribute_name The name of the attribute
	 * @param value The value to retrieve - null if the object id has not been registered in this store
	 * @throws ElementNotFoundException if there is no object with this id
	 **/
	const double& getNumeric(const object_id& oid, const std::string& attribute_name) const;

	/**
	 * @brief Removes all the attribute values from an object.
	 * If the same object is queried after this method has been called, default values will be returned.
	 * @param object_id The id of the object to be removed from the store.
	 **/
	void remove(const object_id& oid);

public:
	/* default values */
	double default_numeric = 0.0;
	std::string default_string = "";
private:
	/* meta-data: names and types of attributes */
	std::vector<AttributeSharedPtr> attribute_vector;
	std::map<std::string,int> attribute_ids;
	/* These maps are structured as: map[AttributeName][object_id][AttributeValue] */
	std::map<std::string, std::map<object_id, std::string> > string_attribute;
	std::map<std::string, std::map<object_id, double> > numeric_attribute;
};

typedef std::shared_ptr<AttributeStore> AttributeStoreSharedPtr;

/**********************************************************************/
/** MLNetwork *********************************************************/
/**********************************************************************/

class MLNetwork {
	friend class actor_list;
	friend class layer_list;
	friend class node_list;
	friend class edge_list;

	/**************************************************************************************
	 * Constructors/destructors
	 **************************************************************************************/
private:
	/**
	 * Creates an empty MLNetwork.
	 * @param name name of the new MLNetwork
	 */
	MLNetwork(const std::string& name);

public:

	static MLNetworkSharedPtr create(const std::string& name);

	/** Default destructor */
	~MLNetwork();

	/**
	 * @return the name of the MLNetwork
	 **/
	std::string name() const;

	/**************************************************************************************
	 * Basic structural operations used to modify a MLNetwork
	 **************************************************************************************/

	/**
	 * @brief Adds a new actor to the MLNetwork.
	 * A new identifier and a default name are automatically associated to the new actor. The default name is guaranteed to be unique
	 * only if no actors have been previously added via the addActor(const std::string&) method.
	 * @return a pointer to the new actor
	 **/
	ActorSharedPtr add_actor();

	/**
	 * @brief Adds a new actor to the MLNetwork.
	 * A new identifier is automatically associated to the new actor.
	 * @param name name of the actor
	 * @return a pointer to the new actor
	 * @throws DuplicateElementException if the actor is already present in the network
	 **/
	ActorSharedPtr add_actor(const std::string& name);

	/**
	 * @brief Returns an actor by ID.
	 * This method can also be used to check if an actor is present in the MLNetwork.
	 * @param id identifier of the actor
	 * @return a pointer to the requested actor, or null if the actor does not exist
	 **/
	ActorSharedPtr get_actor(const actor_id& id) const;

	/**
	 * @brief Returns an actor by name.
	 * This function can also be used to check if an actor is present in the MLNetwork.
	 * @param name name of the actor
	 * @return a pointer to the requested actor, or null if the actor does not exist
	 **/
	ActorSharedPtr get_actor(const std::string& name) const;

	/**
	 * @brief Returns all the actors in the MLNetwork.
	 * @return a pointer to an actor iterator
	 **/
	const ObjectStore<ActorSharedPtr>& get_actors() const;

	/**
	 * @brief Adds a new layer to the MLNetwork.
	 * A new identifier and a default name are automatically associated to the new layer
	 * @return a pointer to the new layer
	 **/
	LayerSharedPtr add_layer(bool directed);

	/**
	 * @brief Sets the default edge directionality depending on the layers of the connected nodes.
	 * @param layer1 a pointer to the "from" layer
	 * @param layer2 a pointer to the "to" layer
	 * @param directed TRUE or FALSE
	 * @return a pointer to the new layer
	 **/
	void set_directed(LayerSharedPtr layer1, LayerSharedPtr layer2, bool directed);

	/**
	 * @brief Gets the default edge directionality depending on the layers of the connected nodes.
	 * @return a Boolean value indicating if edges among these two layers are directed or not.
	 **/
	bool is_directed(LayerSharedPtr layer1, LayerSharedPtr layer2) const;

	/**
	 * @brief Adds a new layer to the MLNetwork.
	 * A new identifier is automatically associated to the new layer
	 * @param name name of the layer
	 * @return a pointer to the new layer
	 * @throws DuplicateElementException if the layer is already present in the network
	 **/
	LayerSharedPtr add_layer(const std::string& name, bool directed);

	/**
	 * @brief Returns a layer by ID.
	 * This function can also be used to check if a layer is present in the MLNetwork
	 * @param id identifier of the layer
	 * @return a pointer to the requested layer, or null if the layer does not exist
	 **/
	LayerSharedPtr get_layer(const layer_id& id) const;

	/**
	 * @brief Returns a layer by name.
	 * This function can also be used to check if a layer is present in the MLNetwork
	 * @param name name of the layer
	 * @return a pointer to the requested layer, or null if the layer does not exist
	 **/
	LayerSharedPtr get_layer(const std::string& name) const;

	/**
	 * @brief Returns all the layers in the MLNetwork.
	 * @return a layer iterator
	 **/
	const ObjectStore<LayerSharedPtr>& get_layers() const;

	/**
	 * @brief Adds a new node to the MLNetwork.
	 * A new identifier and a default name are automatically associated to the new node
	 * @param layer pointer to the layer where this node is located
	 * @param actor pointer to the actor corresponding to this node
	 * @return a pointer to the new node
	 * @throws ElementNotFoundException if the input layer or actor are not present in the network
	 **/
	NodeSharedPtr add_node(const ActorSharedPtr& actor, const LayerSharedPtr& layer);

	/**
	 * @brief Adds a new node to the MLNetwork.
	 * A new identifier is automatically associated to the new node
	 * @param layer pointer to the layer where this node is located
	 * @param actor pointer to the actor corresponding to this node
	 * @param name name of the node
	 * @return a pointer to the new node
	 * @throws ElementNotFoundException if the input layer or actor are not present in the network
	 **/
	NodeSharedPtr add_node(const std::string& name, const ActorSharedPtr& actor, const LayerSharedPtr& layer);

	/**
	 * @brief Returns a node by ID.
	 * This function can also be used to check if a node is present in the MLNetwork
	 * @param id identifier of the layer
	 * @return a pointer to the requested node, or null if the node does not exist
	 **/
	NodeSharedPtr get_node(const node_id& id) const;

	/**
	 * @brief Returns a node by name from a specific layer.
	 * This function can also be used to check if a node is present in the MLNetwork
	 * @param name name of the node
	 * @param layer pointer to the layer where this node is located
	 * @return a pointer to the requested node, or null if the node does not exist
	 **/
	NodeSharedPtr get_node(const std::string& name, const LayerSharedPtr& layer) const;

	/**
	 * @brief Returns all the nodes in the MLNetwork.
	 * @return a node iterator
	 **/
	const ObjectStore<NodeSharedPtr>& get_nodes() const;

	/**
	 * @brief Returns all the nodes in a layer.
	 * @param layer pointer to the layer where this node is located
	 * @return a node iterator
	 **/
	const ObjectStore<NodeSharedPtr>& get_nodes(const LayerSharedPtr& layer) const;

	/**
	 * @brief Returns the nodes associated to the input actor.
	 * @param actor pointer to the actor
	 * @return an iterator containing pointers to nodes
	 **/
	const ObjectStore<NodeSharedPtr>& get_nodes(const ActorSharedPtr& actor) const;

	/**
	 * @brief Adds a new edge to the MLNetwork.
	 * Multiple edges between the same pair of nodes are not allowed. The directionality of the
	 * edge is defined by the layers of the two nodes.
	 * @param node1 a pointer to the "from" node if directed, or to one end of the edge if undirected
	 * @param node2 a pointer to the "to" node if directed, or one end of the edge if undirected
	 * @return a pointer to the new edge
	 * @throws ElementNotFoundException if the input nodes are not present in the network
	 **/
	EdgeSharedPtr add_edge(const NodeSharedPtr& node1, const NodeSharedPtr& node2);

	/**
	 * @brief Returns an edge.
	 * This function can also be used to check if an edge is present in the MLNetwork
	 * @param node1 a pointer to the "from" node if directed, or to one end of the edge if undirected
	 * @param node2 a pointer to the "to" node if directed, or one end of the edge if undirected
	 * @return a pointer to the requested edge, or null if the edge does not exist
	 **/
	EdgeSharedPtr get_edge(const NodeSharedPtr& node1,  const NodeSharedPtr& node2) const;

	/**
	 * @brief Returns all the edges in the MLNetwork.
	 * @return an edge iterator
	 **/
	const ObjectStore<EdgeSharedPtr>& get_edges() const;

	/**
	 * @brief Returns all the edges from a layer A to a layer B.
	 * This method can also retrieve intra-layer edges setting A = B.
	 * @param layer1 pointer to the layer where the first ends of the edges are located
	 * @param layer2 pointer to the layer where the second ends of the edges are located
	 * @return an edge iterator
	 **/
	const ObjectStore<EdgeSharedPtr>& get_edges(const LayerSharedPtr& layer1, const LayerSharedPtr& layer2) const;

	/**
	 * @brief Deletes an existing node.
	 * All related data, including node attributes and edges involving this node, are also deleted.
	 * @param node a pointer to the node to be deleted
	 **/
	void erase(const NodeSharedPtr& node);

	/**
	 * @brief Deletes an existing edge.
	 * Attribute values associated to this edge are also deleted.
	 * @param edge a pointer to the edge to be deleted
	 **/
	void erase(const EdgeSharedPtr& edge);

	// deleteActor and deleteLayer are not considered necessary

	/*****************************************************************************************
	 * Functions returning information about the MLNetwork.
	 *****************************************************************************************/

	/**
	 * @brief Returns the nodes with an edge from/to the input node.
	 * @param node pointer to the node
	 * @param mode IN, OUT or INOUT
	 * @return an iterator containing pointers to nodes
	 * @throws WrongParameterException if mode is not one of IN, OUT or INOUT
	 **/
	const ObjectStore<NodeSharedPtr>& neighbors(const NodeSharedPtr& node, int mode) const;

	/******************************
	 * Attribute handling
	 ******************************/

	/**
	 * @brief Allows the manipulation of feature vectors associated to actors.
	 * This function is not thread-safe.
	 * @return an AttributeStore storing actor features
	 **/
	AttributeStoreSharedPtr actor_features();
	const AttributeStoreSharedPtr actor_features() const;

	/**
	 * @brief Allows the manipulation of feature vectors associated to layers.
	 * This function is not thread-safe.
	 * @return an AttributeStore storing layer features
	 **/
	AttributeStoreSharedPtr layer_features();
	const AttributeStoreSharedPtr layer_features() const;

	/**
	 * @brief Allows the manipulation of feature vectors associated to nodes.
	 * Every layer can associate different features to its nodes, but all nodes in the same layer have the same features
	 * This function is not thread-safe.
	 * @param layer pointer to the layer where the nodes are located
	 * @return an AttributeStore storing node features
	 **/
	AttributeStoreSharedPtr node_features(const LayerSharedPtr& layer);
	const AttributeStoreSharedPtr node_features(const LayerSharedPtr& layer) const;

	/**
	 * @brief Allows the manipulation of feature vectors associated to edges.
	 * Every pair of layers corresponds to different edge features, but all edges between the same pair of layers have the same features
	 * This function is not thread-safe.
	 * @param layer1 pointer to the layer where the first ends of the edges are located
	 * @param layer2 pointer to the layer where the second ends of the edges are located
	 * @return an AttributeStore storing edge features
	 **/
	AttributeStoreSharedPtr edge_features(const LayerSharedPtr& layer1, const LayerSharedPtr& layer2);
	const AttributeStoreSharedPtr edge_features(const LayerSharedPtr& layer1, const LayerSharedPtr& layer2) const;

	/* Output */
	std::string to_string() const;

private:

	// MLNetwork attributes
	std::string mlnet_name;

	// largest identifier assigned so far
	node_id max_node_id;
	edge_id max_edge_id;
	actor_id max_actor_id;
	layer_id max_layer_id;

	/* Edge directionality */
	std::map<layer_id, std::map<layer_id, bool> > edge_directionality;

	/* Indexes on internal entities, by id and by name */
	ObjectStore<LayerSharedPtr> layers_by_id;
	std::map<std::string, LayerSharedPtr> layers_by_name;

	ObjectStore<ActorSharedPtr> actors_by_id;
	std::map<std::string, ActorSharedPtr> actors_by_name;

	ObjectStore<NodeSharedPtr> nodes_by_id;
	std::map<layer_id, ObjectStore<NodeSharedPtr> > nodes_by_layer_and_id;
	std::map<actor_id, ObjectStore<NodeSharedPtr> > nodes_by_actor_and_id;
	std::map<layer_id, std::map<std::string, NodeSharedPtr> > nodes_by_layer_and_name;

	ObjectStore<EdgeSharedPtr> edges_by_id;
	std::map<layer_id, std::map<layer_id, ObjectStore<EdgeSharedPtr> > > edges_by_layers_and_id;
	std::map<node_id, std::map<node_id, EdgeSharedPtr> > edges_by_nodes;
	std::map<node_id, ObjectStore<NodeSharedPtr> >  neighbors_out;
	std::map<node_id, ObjectStore<NodeSharedPtr> > neighbors_in;
	std::map<node_id, ObjectStore<NodeSharedPtr> > neighbors_all;

	/* objects storing the feature vectors of the different components */
	AttributeStoreSharedPtr actor_attributes;
	AttributeStoreSharedPtr layer_attributes;
	std::map<layer_id, AttributeStoreSharedPtr> node_attributes;
	std::map<layer_id, std::map<layer_id, AttributeStoreSharedPtr> > edge_attributes;

};


/**********************************************************************/
/** Distance **********************************************************/
/**********************************************************************/

//class Distance {
//private:
//	const MultipleNetwork *mnet;
//	std::vector<long> num_edges_per_layer;
//	long timestamp;
//	long num_steps;
//
//	friend std::ostream& operator<<(std::ostream &strm, const Distance &dist);
//
//public:
//	/**
//	 * @brief Constructs a
//	 * @throws ElementNotFoundException if the input elements are not present in the network
//	 * @throws OperationNotSupportedException if the two nodes lay on the same network
//	 **/
//	Distance(const Distance& p, long timestamp);
//	Distance(const MultipleNetwork& mnet, long timestamp);
//	Distance(long num_layers, long timestamp);
//	virtual ~Distance();
//
//	long getNumNetworks() const;
//
//	long getTimestamp() const;
//
//	long getNumEdgesOnNetwork(long layer) const;
//
//	void extend(network_id nid);
//
//	bool operator<(const Distance& other) const;
//
//	Distance operator=(const Distance& other) const;
//
//	bool same(const Distance& other) const;
//
//	long length() const;
//};

/**********************************************************************/
/** Path **************************************************************/
/**********************************************************************/

class Path {
private:
	const MLNetworkSharedPtr mnet;
	//std::vector<long> num_edges_per_layer;
	std::vector<EdgeSharedPtr> path;
	NodeSharedPtr origin;
	//long timestamp;

public:
	Path(const MLNetworkSharedPtr mnet, NodeSharedPtr origin);
	/*Path(const Path& p, long timestamp);
	Path(const MLNetwork& mnet, long timestamp);
	Path(long num_layers, long timestamp);
	*/
	~Path();

	NodeSharedPtr begin();

	NodeSharedPtr end();

	//long getTimestamp() const;

	//long getNumNetworks() const;

	//long num_steps(LayerSharedPtr from_layer, LayerSharedPtr to_layer) const;

	//void start(node_id first);

	void step(EdgeSharedPtr e);

	EdgeSharedPtr get_step(long pos) const;

	long length() const;

	bool operator<(const Path& other) const;

	Path operator=(const Path& other) const;

	bool operator==(const Path& other) const;

};

} // namespace mlnet

#endif /* MLNET_DATASTRUCTURES_H_ */
