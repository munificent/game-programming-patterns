//
//  dirty-flag.h
//  cpp
//
//  Created by Bob Nystrom on 9/5/13.
//  Copyright (c) 2013 Bob Nystrom. All rights reserved.
//

#ifndef cpp_dirty_flag_h
#define cpp_dirty_flag_h

namespace DirtyFlag
{
  //^transform
  class Transform
  {
  public:
    static Transform origin();

    Transform combine(const Transform& other) const;
  };
  //^transform

  Transform Transform::origin()
  {
    return Transform();
  }

  Transform Transform::combine(const Transform& other) const
  {
    return other;
  }

  class Mesh;
  static const int MAX_CHILDREN = 16;

  namespace Basic
  {
    //^graph-node
    class GraphNode
    {
    public:
      GraphNode(Mesh* mesh)
      : mesh_(mesh),
        local_(Transform::origin())
      {}
      
    private:
      Transform local_;
      Mesh* mesh_;

      GraphNode* children_[MAX_CHILDREN];
      int numChildren_;
    };
    //^graph-node

    void root()
    {
      //^scene-graph
      GraphNode* graph_ = new GraphNode(NULL);
      // Add children to root graph node...
      //^scene-graph
      use(graph_);
    }
  }

  //^render
  void renderMesh(Mesh* mesh, Transform transform);
  //^render

  void renderMesh(Mesh* mesh, Transform transform)
  {

  }
  
  namespace RenderOnTheFly
  {
    static const int MAX_CHILDREN = 16;
    
    class GraphNode
    {
    public:
      GraphNode(Mesh* mesh)
      : mesh_(mesh),
        local_(Transform::origin())
      {}

      void render(Transform parentWorld);
      
    private:
      Transform local_;
      Mesh* mesh_;

      GraphNode* children_[MAX_CHILDREN];
      int numChildren_;
    };

    //^render-on-fly
    void GraphNode::render(Transform parentWorld) {
      Transform world = local_.combine(parentWorld);

      if (mesh_) renderMesh(mesh_, world);

      for (int i = 0; i < numChildren_; i++) {
        children_[i]->render(world);
      }
    }
    //^render-on-fly

    void root()
    {
      GraphNode* graph_ = new GraphNode(NULL);
      //^render-root
      graph_->render(Transform::origin());
      //^render-root
    }
  }

  namespace Dirty
  {
    //^dirty-graph-node
    class GraphNode
    {
    public:
      GraphNode(Mesh* mesh)
      : mesh_(mesh),
        local_(Transform::origin()),
        dirty_(true)
      {}
      //^omit
      void setTransform(Transform local);
      void render(Transform parentWorld, bool dirty);
      //^omit

      // Other methods...

    private:
      Transform world_;
      bool dirty_;
      // Other fields...
      //^omit
      Transform local_;
      Mesh* mesh_;

      static const int MAX_CHILDREN = 16;
      GraphNode* children_[MAX_CHILDREN];
      int numChildren_;
      //^omit
    };
    //^dirty-graph-node

    //^set-transform
    void GraphNode::setTransform(Transform local) {
      local_ = local;
      dirty_ = true;
    }
    //^set-transform

    //^dirty-render
    void GraphNode::render(Transform parentWorld, bool dirty) {
      dirty |= dirty_;
      if (dirty) {
        world_ = local_.combine(parentWorld);
        dirty_ = false;
      }

      if (mesh_) renderMesh(mesh_, world_);

      for (int i = 0; i < numChildren_; i++) {
        children_[i]->render(world_, dirty);
      }
    }
    //^dirty-render
  }
}

#endif
