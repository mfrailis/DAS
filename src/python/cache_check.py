import hashlib as _hash
import os as _os
from lxml import etree as _et

if __name__ == '__main__':
    import sys

    ddl_types   = sys.argv[1]
    f_signature = sys.argv[2]
    
    if _os.path.exists(f_signature):
        f = open(f_signature,'r')
        old_sig = f.read()
        f.close()
    else:
        old_sig = ""

    tree = _et.parse(ddl_types)
    tree.xinclude()
    serial_tree = _et.tostring(tree)

    new_sig = _hash.sha1(serial_tree).hexdigest()

    if new_sig != old_sig:
        f = open(f_signature,'w')
        f.write(new_sig)
        f.close()
