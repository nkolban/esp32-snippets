#!/bin/bash
# Extract the text and data sections to their own files.
xtensa-esp32-elf-objcopy \
  --dump-section .text=text.dat \
  --dump-section .data=data.dat \
  stub_flasher_32.elf

# Parse out the text load address, the data load address and the entry point from the ELF.
text_address=`xtensa-esp32-elf-objdump --section-headers --wide --section=.text stub_flasher_32.elf | sed -n '$s/^\s*[[:alnum:]]*\s*\S*\s*[[:alnum:]]*\s*[[:alnum:]]*\s*\([[:alnum:]]*\).*/\1/p'`
#echo "Text address: 0x${text_address}"
data_address=`xtensa-esp32-elf-objdump --section-headers --wide --section=.data stub_flasher_32.elf | sed -n '$s/^\s*[[:alnum:]]*\s*\S*\s*[[:alnum:]]*\s*[[:alnum:]]*\s*\([[:alnum:]]*\).*/\1/p'`
#echo "Data address: 0x${data_address}"
entry_point=`xtensa-esp32-elf-objdump --wide --file-headers stub_flasher_32.elf | sed -n '/start address/s/start address 0x\([[:alnum:]]\)/\1/p'`
#echo "Entry point:  0x${entry_point}"

echo "{"
echo "  \"textAddress\": \"${text_address}\","
echo "  \"dataAddress\": \"${data_address}\","
echo "  \"entryPoint\":  \"${entry_point}\","
echo "  \"textData\":    \"$(base64 --wrap=0 text.dat)\","
echo "  \"dataData\":    \"$(base64 --wrap=0 data.dat)\""
echo "}"

# Remove the temporary text and data files.
#rm text.dat data.dat
