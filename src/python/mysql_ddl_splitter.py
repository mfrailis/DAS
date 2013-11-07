import re
import sys
import hashlib as _hash
            
if __name__ == '__main__':

    f_name     = sys.argv[1]
    ddl_fname  = sys.argv[2]
    cnst_fname = sys.argv[3]
    
    f = open(f_name, 'r')
    text = f.read()
    f.close()
    
    constraints = re.findall('ALTER TABLE[a-zA-Z0-9\ \`\,_\(\)\n]*;',text)

    ddl = text

    for cs in constraints:
        ddl = ddl.replace(cs,'')
    
    f = open(ddl_fname, 'w')
    f.write(ddl)
    f.close()

    hash_constraints = []
    for cs in constraints:
        cl = re.findall('ADD CONSTRAINT \`[a-zA-Z0-9\_]*\`',cs)
        for i in cl:
            hc = 'ADD CONSTRAINT `fk_'+_hash.sha1(i).hexdigest()+'`'
            cs = cs.replace(i,hc)

        hash_constraints.append(cs)

    f = open(cnst_fname, 'w')
    f.writelines(hash_constraints)
    f.close()

