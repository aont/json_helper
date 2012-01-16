#include <cstdio>

#include <json.hpp>

int main()
{

#if false
  json::reader reader;
  reader.fopen("data.js");

  while(true) {

    int vtype = reader.next();
    printf("type: %d %c\n", vtype, vtype);
    if(vtype=='s') {
      printf("read: %s\n",
	     reader.get<const char*>() );
    } else if(vtype==EOF) {
      break;
    }  
    
  }
    
  reader.fclose();
#else

  json::writer writer;
  sprintf(writer.indent_str, "  ");
  //writer.if_compress = true;
  writer.set_file(stdout);

  writer.write('{');
  {

    writer.write("hello");
    writer.write(':');

    writer.write('{');
    {
      writer.write_string("hello");
      writer.write(':');
      writer.write_string("hi");

      writer.write(',');

      writer.write("hello2");
      writer.write(':');
      writer.write("hi");
    }
    writer.write('}');

    writer.write(',');

    writer.write("he\tllo2");
    writer.write(':');

    writer.write('[');
    {
      writer.write("he\nllo");

      writer.write(',');

      writer.write((const double)123.45);

    }
    writer.write(']');

  }
  writer.write('}');

  printf("\n");

#endif

  return 0;
}
