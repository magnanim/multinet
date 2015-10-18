/*
 * testMultilayerNetwork.cpp
 *
 * Author: Matteo Magnani <matteo.magnani@it.uu.se>
 * Version: 0.0.1
 */

#import "test.h"
#import "datastructures.h"
#import "exceptions.h"
#import "utils.h"
#import <array>

using namespace mlnet;

void testMLNetwork() {
	log("TESTING basic MLNetwork components (node, edge, layer, actor)",true);
	// These are normally automatically created by functions of the MultilayerNetwork class.
	log("...creating two actors...",false);
	ActorSharedPtr actor1(new actor(1,"Matteo"));
	ActorSharedPtr actor2(new actor(2,"Luca"));
	log("done!");
	log("...creating two layers...",false);
	LayerSharedPtr layer1(new layer(1,"Facebook"));
	LayerSharedPtr layer2(new layer(2,"Twitter"));
	log("done!");
	log("...creating four nodes...",false);
	NodeSharedPtr node1(new node(1,actor1,layer1));
	NodeSharedPtr node2(new node(2,actor1,layer2));
	NodeSharedPtr node3(new node(3,actor1,layer2));
	NodeSharedPtr node4(new node(4,actor2,layer1));
	log("done!");
	log("...creating five edges...",false);
	EdgeSharedPtr edge1(new edge(1,node1,node2,true)); // directed
	EdgeSharedPtr edge2(new edge(2,node2,node1,true)); // directed
	EdgeSharedPtr edge3(new edge(3,node1,node2,false)); // indirected
	EdgeSharedPtr edge4(new edge(3,node2,node1,false)); // indirected
	if (*edge1==*edge2) throw FailedUnitTestException("Wrong edge comparison");
	if (*edge2==*edge3) throw FailedUnitTestException("Wrong edge comparison");
	if (*edge3!=*edge4) throw FailedUnitTestException("Wrong edge comparison");
	log("done!");
	log("TEST SUCCESSFULLY COMPLETED (basic MLNetwork components)");



	log("******************************************");
	log("TESTING MLNetwork");
	log("Creating an empty ML network...",false);
	MLNetworkSharedPtr mnet = MLNetwork::create("friends");
	log(mnet->to_string() + " done!");
	log("Adding three actors...",false);
	ActorSharedPtr a1 = mnet->add_actor("a1");
	ActorSharedPtr a2 = mnet->add_actor("a2");
	ActorSharedPtr a3 = mnet->add_actor("Matteo");
	//if ((*mnet->getActor(3))!=(*a2)) throw FailedUnitTestException("Could not retrieve actor");
	if ((*mnet->get_actor("Matteo"))!=(*a3)) throw FailedUnitTestException("Could not retrieve actor");
	int num_actors=0;
	for (ActorSharedPtr actor : mnet->get_actors()) {
		num_actors++;
		log(actor->to_string() + " ",false);
	}
	if (num_actors!=3 || num_actors!=mnet->get_actors().size()) throw FailedUnitTestException("Could not retrieve all actors");
	log("done!");
	log("Adding duplicate actors (should fail)...",false);
	// the following instruction should generate an error: the actor already exists
	try {
		mnet->add_actor("Matteo");
		// should not arrive here
		throw FailedUnitTestException("Duplicate actor insertion not caught");
	}
	catch (DuplicateElementException& ex) {
		log("[FAIL] ",false);
	}
	log("done!");

	log("Adding three layers: ",false);
	LayerSharedPtr l1 = mnet->add_layer("l1",false);
	LayerSharedPtr l2 = mnet->add_layer("l2",false);
	LayerSharedPtr l3 = mnet->add_layer("Facebook",true);
	mnet->set_directed(l2,l3,true);
	if ((*mnet->get_layer("Facebook"))!=(*l3)) throw FailedUnitTestException("Could not retrieve layer");
	int num_layers=0;
	for (LayerSharedPtr layer : mnet->get_layers()) {
		num_layers++;
		log(layer->to_string() + " ",false);
	}
	if (num_layers!=3 || num_layers!=mnet->get_layers().size()) throw FailedUnitTestException("Could not retrieve all layers");
	log("");
	log("done!");

	// the following instruction should fail: the layer already exists
	try {
		mnet->add_layer("Facebook",false);
		// should not arrive here
		throw FailedUnitTestException("Duplicate layer insertion not caught");
	}
	catch (DuplicateElementException& ex) {
		log("[FAIL] ",false);
	}
	log("done!");

	log("Adding 9 nodes: ",false);
	// Layer 1
	NodeSharedPtr n1v0 = mnet->add_node(a1,l1);
	NodeSharedPtr n1v1 = mnet->add_node(a2,l1);
	NodeSharedPtr n1v2 = mnet->add_node(a3,l1);
	// Layer 2
	NodeSharedPtr n2v0 = mnet->add_node(a1,l2);
	NodeSharedPtr n2v1 = mnet->add_node(a2,l2);
	NodeSharedPtr n2v2 = mnet->add_node(a3,l2);
	// Layer 3
	NodeSharedPtr n3v0 = mnet->add_node(a1,l3);
	NodeSharedPtr n3v1 = mnet->add_node(a2,l3);
	NodeSharedPtr n3v2 = mnet->add_node(a3,l3);

	int num_nodes=0;
	for (NodeSharedPtr node : mnet->get_nodes()) {
		num_nodes++;
		log(node->to_string() + " ",false);
	}
	if (num_nodes!=9 || num_nodes!=mnet->get_nodes().size()) throw FailedUnitTestException("Could not retrieve all nodes");
	log("");
	log("done!");

	log("Adding five intra-layer edges and one inter-layer edge: ",false);
	EdgeSharedPtr e1 = mnet->add_edge(n1v0,n1v1);
	EdgeSharedPtr e2 = mnet->add_edge(n2v0,n2v1);
	EdgeSharedPtr e3 = mnet->add_edge(n2v1,n2v2);
	EdgeSharedPtr e4 = mnet->add_edge(n3v0,n3v2);
	EdgeSharedPtr e5 = mnet->add_edge(n3v2,n3v1);

	EdgeSharedPtr e6 = mnet->add_edge(n2v2,n3v1);

	int num_edges=0;
	for (EdgeSharedPtr edge : mnet->get_edges()) {
		num_edges++;
		log(edge->to_string() + " ",false);
	}
	if (num_edges!=6 || num_edges!=mnet->get_edges().size()) throw FailedUnitTestException("Could not retrieve all nodes");
	log("done!");

	log("TESTING attribute management");

	mnet->node_features(l1)->add("weight",NUMERIC_TYPE);
	mnet->node_features(l1)->add("type",STRING_TYPE);
	mnet->edge_features(l1,l2)->add("weight",NUMERIC_TYPE);
	mnet->node_features(l1)->setNumeric(n1v0->id,"weight",32.4);
	mnet->node_features(l1)->setString(n1v0->id,"type","pro");
	if (mnet->node_features(l1)->getNumeric(n1v0->id,"weight")!=32.4) throw FailedUnitTestException("Could not retrieve previously set attribute");
	if (mnet->node_features(l1)->getString(n1v0->id,"type")!="pro") throw FailedUnitTestException("Could not retrieve previously set attribute");
	log("Attributes created for nodes on layer " + l1->to_string() + ":");
	for (AttributeSharedPtr attr: mnet->node_features(l1)->attributes()) {
		log("- Attribute \"" + attr->name()+ "\", type: " + attr->type_as_string());
	}
	log("Attributes created for edges from layer " + l1->to_string() + " to layer "  + l2->to_string() + ":");
	for (AttributeSharedPtr attr: mnet->edge_features(l1,l2)->attributes()) {
		log("- Attribute \"" + attr->name()+ "\", type: " + attr->type_as_string());
	}
	log("Attributes created for edges from layer " + l2->to_string() + " to layer "  + l1->to_string() + " (none expected):");
	for (AttributeSharedPtr attr: mnet->edge_features(l2,l1)->attributes()) {
		log("- Attribute \"" + attr->name()+ "\", type: " + attr->type_as_string());
	}
	log("done!");

	log("Getting in-neighbors: ",false);
	int num_neighbors=0;
	for (NodeSharedPtr node : mnet->neighbors(n3v2,IN)) {
		num_neighbors++;
		log(node->to_string() + " ",false);
	}
	if (num_neighbors!=1) throw FailedUnitTestException("Could not retrieve neighbors");
	log("");
	log("Getting out-neighbors: ",false);
	num_neighbors=0;
	for (NodeSharedPtr node : mnet->neighbors(n3v2,OUT)) {
		num_neighbors++;
		log(node->to_string() + " ",false);
	}
	if (num_neighbors!=1) throw FailedUnitTestException("Could not retrieve neighbors");
	log("done!");
	log("Getting in/out-neighbors: ",false);
	num_neighbors=0;
	for (NodeSharedPtr node : mnet->neighbors(n3v2,INOUT)) {
		num_neighbors++;
		log(node->to_string() + " ",false);
	}
	if (num_neighbors!=2) throw FailedUnitTestException("Could not retrieve neighbors");
	log("done!");
	log("Getting out-neighbors with undirected edges: ",false);
	num_neighbors=0;
	for (NodeSharedPtr node : mnet->neighbors(n2v1,OUT)) {
		num_neighbors++;
		log(node->to_string() + " ",false);
	}
	if (num_neighbors!=2) throw FailedUnitTestException("Could not retrieve neighbors");
	log("done!");
	log("Getting in/out-neighbors with undirected edges: ",false);
	num_neighbors=0;
	for (NodeSharedPtr node : mnet->neighbors(n2v1,INOUT)) {
		num_neighbors++;
		log(node->to_string() + " ",false);
	}
	if (num_neighbors!=2) throw FailedUnitTestException("Could not retrieve neighbors");
	log("done!");

	log("Erasing components: ",false);
	mnet->erase(n3v2);
	if (8 != mnet->get_nodes().size()) throw FailedUnitTestException("Could not retrieve all nodes");
	mnet->erase(e3);
	if (3 != mnet->get_edges().size()) throw FailedUnitTestException("Could not retrieve all edges");
	mnet->erase(a1);
	if (2 != mnet->get_actors().size()) throw FailedUnitTestException("Could not retrieve all actor");
	if (5 != mnet->get_nodes().size()) throw FailedUnitTestException("Could not retrieve all nodes");
	mnet->erase(l1);
	log(mnet->to_string());
	if (2 != mnet->get_layers().size()) throw FailedUnitTestException("Could not retrieve all layers");
	if (3 != mnet->get_nodes().size()) throw FailedUnitTestException("Could not retrieve all nodes");
	log("done!");
	log("TEST SUCCESSFULLY COMPLETED (MLNetwork)");
}


