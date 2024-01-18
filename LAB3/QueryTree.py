
import re
kv={'employee':{'ename','essn','address','salary','superssn','dno'},
    'department':{'dname','dno','mgrssn','mgrstartdate'},
    'project':{'pname','pno','plocation','dno'},
    'works_on':{'essn','pno','hours'}}
class Tree:
    def __init__(self,root):
        self.root=root
    def __repr__(self) -> str:
        return self.DFS(self.root)
    def DFS(self,node):
        s=""
        s+=" "*node.level*4
        s+=node.__repr__()
        s+="\n"
        for n in node.children:
            s+=self.DFS(n)
        return s

class Node:
    def __init__(self):
        self.father=None
        self.children=[]
        self.level=0
class Select(Node):
    def __init__(self,condition):
        '''
        condition=((k1,v1),(k2,v2),and/or)
        '''
        super(Select, self).__init__()
        self.condition=condition
        cond_list=condition.split('&')
        self.cond_dict={}
        for c in cond_list:
            k,v=c.split('=')
            
            self.cond_dict[k.strip()]=v.strip()
        self.attr_list=self.cond_dict.keys()
        


    def __repr__(self) -> str:
        return f"Select({self.condition})"
    
class Projection(Node):
    def __init__(self,attr_list):
        '''
        condition=((k1,v1),(k2,v2),and/or)
        '''
        super(Projection, self).__init__()
        self.attr_list=attr_list


    def __repr__(self) -> str:
        return f"Project{self.attr_list}"

class Join(Node):
    def __init__(self):
        super(Join, self).__init__()
    
    def __repr__(self) -> str:
        return f"JOIN"
    
class Relation(Node):
    def __init__(self,name):
        super(Relation,self).__init__()
        self.name=name
        self.attr_list=kv[self.name]
    def __repr__(self) -> str:
        return self.name
    
def parsing(query):
    query=query.lower()
    str_list=re.split(",|\s",query)
    while "" in str_list:
        str_list.remove("")
    i=0
    op_list=[]
    while i<len(str_list):
        if str_list[i]=='select':
            j=i+1
            while str_list[j]!=']':
                j+=1
            cond=' '.join(str_list[i+2:j])
            sel=Select(cond)
            op_list.append(sel)
            i=j+1
        elif str_list[i]=='projection':
            j=i+1
            while str_list[j]!=']':
                j+=1
            cond=str_list[i+2:j]
            prj=Projection(cond)
            op_list.append(prj)
            i=j+1
        elif str_list[i]=='join':
            j=Join()
            r1=Relation(str_list[i-1])
            r2=Relation(str_list[i+1])
            op_list.append(j)
            op_list.append(r1)
            op_list.append(r2)
            break
        i+=1
    tab=0
    for i,op in enumerate(op_list):
        if isinstance(op,Select) or isinstance(op,Projection):
            op.children.append(op_list[i+1])
            op_list[i+1].level=op.level+1
            op_list[i+1].father=op
        elif isinstance(op,Join):
            op.children.append(op_list[i+1])
            op.children.append(op_list[i+2])
            op_list[i+1].level=op.level+1
            op_list[i+2].level=op.level+1
            op_list[i+1].father=op
            op_list[i+2].father=op
    tree=Tree(op_list[0])
    print(tree)
    return tree,op_list
def set_depth(node,tab):
    node.level=tab
    for n in node.children:
        set_depth(n,tab+1)
def optimize(query):
    query=query.lower()
    print("优化前：")
    tree,op_list=parsing(query)
    tmp=tree.root
    tree.root=tree_optimizer(tmp)
    set_depth(tree.root,0)
    print("优化后：")
    print(tree)

def tree_optimizer(node:Node):
    new_children=[]
    for n in node.children:
        t=tree_optimizer(n)
        t.father=node
        new_children.append(t)
    node.children=new_children
    if isinstance(node,Projection) and isinstance(node.children[0],Join):
        j_tmp=node.children[0]
        in_r1=[]
        in_r2=[]
        t1=j_tmp.children[0]
        while not isinstance(t1,Relation):
            t1=t1.children[0]
        t2=j_tmp.children[1]
        while not isinstance(t2,Relation):
            t2=t2.children[0]
        for attr in node.attr_list:
            if attr in t1.attr_list:
                in_r1.append(attr)
            if attr in t2.attr_list:
                in_r2.append(attr)
        for i in in_r1:
            while i in node.attr_list:
                node.attr_list.remove(i)
        for i in in_r2:
            while i in node.attr_list:
                node.attr_list.remove(i)
        if in_r1:
            p1=Projection(in_r1)
            p1.children.append(j_tmp.children[0])
            p1.level=j_tmp.children[0].level
            j_tmp.children[0].level+=1
            j_tmp.children[0].father=p1
        else:
            p1=j_tmp.children[0]
        if in_r2:
            p2=Projection(in_r2)
            p2.children.append(j_tmp.children[1])
            p2.level=j_tmp.children[1].level
            j_tmp.children[1].level+=1
            j_tmp.children[1].father=p2
        else:
            p2=j_tmp.children[1]
        j_tmp.children=[p1,p2]
        p1.father=j_tmp
        p2.father=j_tmp
        j_tmp.father=None
        if node.attr_list:
            return node
        return j_tmp
    if isinstance(node,Select) and isinstance(node.children[0],Join):
        j_tmp=node.children[0]
        in_r1=[]
        in_r2=[]
        for attr in node.cond_dict.keys():
            if attr in j_tmp.children[0].attr_list:
                in_r1.append(attr)
            if attr in j_tmp.children[1].attr_list:
                in_r2.append(attr)
        if in_r1:
            s_tmp=""
            for a in in_r1:
                s_tmp+=" ".join([a,'=',node.cond_dict[a]])
            p1=Select(s_tmp)
            p1.children.append(j_tmp.children[0])
            p1.level=j_tmp.children[0].level
            j_tmp.children[0].level+=1
            j_tmp.children[0].father=p1
        else:
            p1=j_tmp.children[0]
        if in_r2:
            s_tmp=""
            for a in in_r2:
                s_tmp+=" ".join([a,'=',node.cond_dict[a]])
            p2=Select(s_tmp)
            p2.children.append(j_tmp.children[1])
            p2.level=j_tmp.children[1].level
            j_tmp.children[1].level+=1
            j_tmp.children[1].father=p2
        else:
            p2=j_tmp.children[1]
        j_tmp.children=[p1,p2]
        p1.father=j_tmp
        p2.father=j_tmp
        j_tmp.father=None
        return j_tmp
    return node
    
if __name__=="__main__":
    q1="SELECT [ ENAME = ’Mary’ & DNAME = ’Research’ ] ( EMPLOYEE JOIN DEPARTMENT )"
    q2="PROJECTION [ BDATE ] ( SELECT [ ENAME = ’John’ & DNAME = ’ Research’ ] ( EMPLOYEE JOIN DEPARTMENT ) )"
    q3="SELECT [ ESSN = ’01’ ] (  PROJECTION [ ESSN, PNAME ] ( WORKS_ON JOIN PROJECT ) )"
    q=[q1,q2,q3]
    for qi in q:
        optimize(qi)