
#include "cAnalyzeGenotypeTreeStats.h"

#include "cAnalyzeGenotype.h"
#include "tHashTable.h"

void cAnalyzeGenotypeTreeStats::AnalyzeBatchTree(tList<cAnalyzeGenotype> &genotype_list){
  cAnalyzeGenotype * genotype = NULL;
  tListIterator<cAnalyzeGenotype> batch_it(genotype_list);
  const int num_gens = genotype_list.GetSize();

  int array_pos = 0;

  /*
  Put all of the genotypes in an array for easy reference and collect other information on them as we process them. {{{4
  */
  tArray<cAnalyzeGenotype *> gen_array(num_gens);
  tHashTable<int, int> id_hash;  // Store array pos for each id.
  tArray<int> id_array(num_gens), pid_array(num_gens);
  tArray<int> depth_array(num_gens), birth_array(num_gens);

  array_pos = 0;
  batch_it.Reset();
  while ((genotype = batch_it.Next()) != NULL) {
    id_hash.Add(genotype->GetID(), array_pos);
    array_pos++;
  }

  m_agl.Resize(num_gens);
  array_pos = 0;
  batch_it.Reset();
  while ((genotype = batch_it.Next()) != NULL) {
    // Put the genotype in an array.
    m_agl[array_pos].genotype = genotype;
    m_agl[array_pos].id = genotype->GetID();
    m_agl[array_pos].pid = genotype->GetParentID();
    m_agl[array_pos].depth = genotype->GetDepth();
    m_agl[array_pos].birth = genotype->GetUpdateBorn();
    array_pos++;
  }

  //// Now collect information about the offspring of each individual. {{{4
  tArray<int> ppos_array(num_gens), offspring_count(num_gens);
  offspring_count.SetAll(0);

  // For each genotype, figure out how far back you need to go to get to a branch point. {{{4
  tArray<int> anc_branch_dist_array(num_gens);
  tArray<int> anc_branch_pos_array(num_gens);
  anc_branch_dist_array.SetAll(-1);
  anc_branch_pos_array.SetAll(-1);
  bool found = true;
  int loop_count = 0;

  /*
  Link each offspring to its parent. {{{4
  */
  for (int pos = 0; pos < num_gens; pos++) {
    //cAnalyzeGenotype * genotype = gen_array[pos];
    cAnalyzeGenotype * genotype = m_agl[pos].genotype;
    int parent_id = genotype->GetParentID();
    if (-1 != parent_id){
      id_hash.Find(parent_id, m_agl[pos].ppos);
      int parent_position = m_agl[pos].ppos;
      m_agl[parent_position].offspring_positions.Push(pos);
      /* XXX I think I'll be able to remove this. */
      cAnalyzeGenotype * parent_genotype = m_agl[parent_position].genotype;
      genotype->LinkParent(parent_genotype);
    }
  }

  /*
  Count offspring of each parent. {{{4
  */
  for (int pos = 0; pos < num_gens; pos++) {
    m_agl[pos].offspring_count = m_agl[pos].offspring_positions.GetSize();
  }


  /*
  For each genotype, figure out how far back you need to go to get to a branch point. {{{4
  */
  found = true;
  loop_count = 0;
  while (found == true) {
    found = false;
    for (int pos = 0; pos < num_gens; pos++) {
      if (m_agl[pos].anc_branch_dist > -1) continue; // continue if its set.
      found = true;
      int parent_pos = m_agl[pos].ppos;
      if (parent_pos == -1) {
        m_agl[pos].anc_branch_dist = 0;  // Org is root.
      } else if (m_agl[parent_pos].offspring_count > 1) {        // Parent is branch.
        m_agl[pos].anc_branch_dist = 1;
        m_agl[pos].anc_branch_id = m_agl[parent_pos].id;
        m_agl[pos].anc_branch_pos = parent_pos;
      } else if (m_agl[parent_pos].anc_branch_dist > -1) {     // Parent calculated.
        m_agl[pos].anc_branch_dist = m_agl[parent_pos].anc_branch_dist + 1;
        m_agl[pos].anc_branch_id = m_agl[parent_pos].anc_branch_id;
        m_agl[pos].anc_branch_pos = m_agl[parent_pos].anc_branch_pos;
      }
      // Otherwise, we are not yet ready to calculate this entry.
    }
    loop_count++;
  }

  // compute number of subtree nodes.
  int branch_tree_size = 0;
  for (int pos = 0; pos < num_gens; pos++) {
    if(m_agl[pos].offspring_count != 1){
      branch_tree_size++;
    }
  }

  m_agl2.Resize(branch_tree_size);  // Store agl data for each id.
  tHashTable<int, int> id_hash_2;
  int array_pos_2 = 0;
  if (true) for (int pos = 0; pos < num_gens; pos++) {
    int offs_count = m_agl[pos].offspring_count;
    if (offs_count != 1){
      m_agl2[array_pos_2].id = m_agl[pos].id;
      m_agl2[array_pos_2].pid = m_agl[pos].pid;
      m_agl2[array_pos_2].depth = m_agl[pos].depth;
      m_agl2[array_pos_2].birth = m_agl[pos].birth;
      m_agl2[array_pos_2].anc_branch_dist = m_agl[pos].anc_branch_dist;
      m_agl2[array_pos_2].anc_branch_id = m_agl[pos].anc_branch_id;
      /*
      missing still are ppos (skip this), offspring_count (redundant),
      anc_branch_pos, off_branch_dist_acc (to be calculated),
      offspring_positions
      */
      id_hash_2.Add(m_agl2[array_pos_2].id, array_pos_2);
      array_pos_2++;
    }
  }

  // find branch ancestor positions. {{{4
  for (int pos = 0; pos < branch_tree_size; pos++){
    int anc_branch_id = m_agl2[pos].anc_branch_id;
    id_hash_2.Find(anc_branch_id, m_agl2[pos].anc_branch_pos);
    int anc_branch_pos = m_agl2[pos].anc_branch_pos;
    if(0 <= anc_branch_pos){
      m_agl2[anc_branch_pos].offspring_positions.Push(pos);
    }
  }
  
  /*
  For DFS of branch tree, locate root. {{{4
  */
  cAGLData *root = 0;
  for (int pos = 0; pos < branch_tree_size; pos++){
    m_agl2[pos].traversal_visited = false;
    /*
    root : anc_branch_dist: 0 anc_branch_id: -1 anc_branch_pos: -1 

    Only one of these conditions should be needed. I'm mixing-in a
    sanity check here. I ought to move it...
    */
    if( (m_agl2[pos].anc_branch_dist == 0)
      ||(m_agl2[pos].anc_branch_id == -1)
      ||(m_agl2[pos].anc_branch_pos == -1)
    ){
      root = &(m_agl2[pos]);
      /* Sanity check. */
      if(!( (m_agl2[pos].anc_branch_dist == 0)
          &&(m_agl2[pos].anc_branch_id == -1)
          &&(m_agl2[pos].anc_branch_pos == -1)
        )
      ){
        // FIXME : ERROR("while looking for root of subtree, found inconsistencies -");
        return;
      }
    }
  }

  /*
  DFS of branch tree, to accumulate branch distances. {{{4
  */
  tList<cAGLData> dfs_stack;
  if(0 != root){
    /* DFS. */
    dfs_stack.Push(root);
    cAGLData *node = 0;
    while (0 < dfs_stack.GetSize()){
      node = dfs_stack.Pop();
      if (0 != node){
        if (! node->traversal_visited){
          dfs_stack.Push(node);
          node->off_branch_dist_acc = 0;
          for (int i = 0; i < node->offspring_positions.GetSize(); i++){
            int pos = node->offspring_positions[i];
            if (! m_agl2[pos].traversal_visited){
              dfs_stack.Push(&(m_agl2[pos]));
            }
          }
          node->traversal_visited = true;
        } else {
          /*
          Child nodes, if any, have been visited and have added their
          off_branch_dist_acc to this node.
          */
          if(0 <= node->anc_branch_pos){
            /*
            Only accumulate to parent if there is a parent (i.e., this
            is not the root.)
            */
            m_agl2[node->anc_branch_pos].off_branch_dist_acc += node->anc_branch_dist;
            m_agl2[node->anc_branch_pos].off_branch_dist_acc += node->off_branch_dist_acc;
          }
        }
      }
    }
  } else {
    // FIXME : ERROR("couldn't find root of subtree -");
    return;
  }

  /*
  Compute cumulative stemminesses. {{{4
  */
  for (int pos = 0; pos < branch_tree_size; pos++){
    if (0 == m_agl2[pos].anc_branch_dist){
      /* Correct stemminess for root... */
      /*
      Let me rephrase that. If anc_branch_dist is zero and all is well,
      we're dealing with the root.
      */
      m_agl2[pos].cumulative_stemminess = 0.0;
    } else {
      m_agl2[pos].cumulative_stemminess =
        (double)(m_agl2[pos].anc_branch_dist)
        /
        (
          (double)(m_agl2[pos].off_branch_dist_acc) + (double)(m_agl2[pos].anc_branch_dist)
        )
        ;
    }
  }

  /*
  Compute average cumulative stemminess. {{{4
  */
  m_stemminess_sum = 0.0;
  m_average_stemminess = 0.0;
  m_inner_nodes = 0;
  if (1 < branch_tree_size) {
    for (int pos = 0; pos < branch_tree_size; pos++){
      bool not_leaf = true;
      if (m_should_exclude_leaves) {
        not_leaf = (0 < m_agl2[pos].off_branch_dist_acc);
      }
      bool not_root = (0 < m_agl2[pos].anc_branch_id);
      if (not_leaf && not_root){
        m_stemminess_sum += m_agl2[pos].cumulative_stemminess;
        m_inner_nodes++;
      }
    }
  }
  if(0 < m_inner_nodes){
    m_average_stemminess = m_stemminess_sum / (m_inner_nodes);
  }

  /*
  Print branch tree. {{{4
  Use to get expected data for making following test.
  */

  if (true){
    for (int pos = 0; pos < branch_tree_size; pos++){
      cout << "id: " << m_agl2[pos].id;
      cout << " offspring_count: " << m_agl2[pos].offspring_positions.GetSize();
      cout << " anc_branch_id: " << m_agl2[pos].anc_branch_id;
      cout << " anc_branch_dist: " << m_agl2[pos].anc_branch_dist;
      cout << " off_branch_dist_acc: " << m_agl2[pos].off_branch_dist_acc;
      cout << " cumulative_stemminess: " << m_agl2[pos].cumulative_stemminess;
      cout << endl;
      if (0 < m_agl2[pos].offspring_positions.GetSize()) {
        cout << "  offspring ids:";
        for (int i = 0; i < m_agl2[pos].offspring_positions.GetSize(); i++){
          cout << " " << m_agl2[pos].offspring_positions[i];
        }
        cout << endl;
      }
    }
    cout << "m_stemminess_sum: " << m_stemminess_sum << endl;
    cout << "m_average_stemminess: " << m_average_stemminess << endl;
    cout << "m_inner_nodes: " << m_inner_nodes << endl;
  }
}