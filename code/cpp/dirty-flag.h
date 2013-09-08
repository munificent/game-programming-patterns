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

  namespace Basic
  {
    //^graph-node
    class GraphNode
    {
    public:
      GraphNode(Mesh* mesh)
      : _mesh(mesh),
        _local(Transform::origin())
      {}
      
    private:
      Transform _local;
      Mesh* _mesh;

      static const int MAX_CHILDREN = 16;
      GraphNode* _children[MAX_CHILDREN];
      int _numChildren;
    };
    //^graph-node

    void root()
    {
      //^scene-graph
      GraphNode* _graph = new GraphNode(NULL);
      // Add children to root graph node...
      //^scene-graph
      use(_graph);
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
    class GraphNode
    {
    public:
      GraphNode(Mesh* mesh)
      : _mesh(mesh),
      _local(Transform::origin())
      {}

      void render(Transform parentWorld);
      
    private:
      Transform _local;
      Mesh* _mesh;

      static const int MAX_CHILDREN = 16;
      GraphNode* _children[MAX_CHILDREN];
      int _numChildren;
    };

    //^render-on-fly
    void GraphNode::render(Transform parentWorld) {
      Transform world = _local.combine(parentWorld);

      if (_mesh) renderMesh(_mesh, world);

      for (int i = 0; i < _numChildren; i++) {
        _children[i]->render(world);
      }
    }
    //^render-on-fly


    void root()
    {
      GraphNode* _graph = new GraphNode(NULL);
      //^render-root
      _graph->render(Transform::origin());
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
      : _mesh(mesh),
        _local(Transform::origin()),
        _dirty(true)
      {}
      //^omit
      void setTransform(Transform local);
      void render(Transform parentWorld, bool dirty);
      //^omit

    private:
      Transform _world;
      bool _dirty;
      // Other fields...
      //^omit
      Transform _local;
      Mesh* _mesh;

      static const int MAX_CHILDREN = 16;
      GraphNode* _children[MAX_CHILDREN];
      int _numChildren;
      //^omit
    };
    //^dirty-graph-node

    //^set-transform
    void GraphNode::setTransform(Transform local) {
      _local = local;
      _dirty = true;
    }
    //^set-transform

    //^dirty-render
    void GraphNode::render(Transform parentWorld, bool dirty) {
      dirty |= _dirty;
      if (dirty) {
        _world = _local.combine(parentWorld);
        _dirty = false;
      }

      if (_mesh) renderMesh(_mesh, _world);

      for (int i = 0; i < _numChildren; i++) {
        _children[i]->render(_world, dirty);
      }
    }
    //^dirty-render
  }
}

#endif
