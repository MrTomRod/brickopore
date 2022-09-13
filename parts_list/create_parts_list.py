from urllib.request import urlopen

# read parts list from brickopore.co.uk
with urlopen('http://www.brickopore.co.uk/brickopore_mk2_parts.csv') as f:
    part_list_csv = f.read().decode('utf-8')

# The BrickLink xml format is documented here: https://www.bricklink.com/help.asp?helpID=207
XML_TEMPLATE = '''
    <ITEM>
        <ITEMTYPE>P</ITEMTYPE>
        <ITEMID>{ITEMID}</ITEMID>
        <COLOR>{COLOR}</COLOR>
        <MINQTY>{MINQTY}</MINQTY>
        <REMARKS>{REMARKS}</REMARKS>
        <NOTIFY>N</NOTIFY>
    </ITEM>
'''[1:]

XML = '<INVENTORY>\n'

for line in part_list_csv.split('\n')[1:]:  # [1:] removes header
    line = line.rstrip().split('\t')
    if len(line) != 10:
        break

    BLItemNo, ElementId, LdrawId, PartName, BLColorId, LDrawColorId, ColorName, ColorCategory, Qty, Weight = line
    XML += XML_TEMPLATE.format(
        ITEMID=BLItemNo,
        COLOR=BLColorId,
        MINQTY=Qty,
        REMARKS=f'{PartName} ({ColorName})',
    )

XML += '</INVENTORY>'

# write file
with open('bricklink-order.xml', 'w') as f:
    f.write(XML)
