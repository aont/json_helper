#ifndef JSON_HPP
#define JSON_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

namespace json
{

  class reader
  {

  public:
    reader()
    {
      this->fp = NULL;
    }

  private:
    FILE* fp;

  public:
    void fopen(const char* const fn)
    {
      if(fp!=NULL) {
	std::fprintf(stderr, "already opening a file\n");
	throw;
      }

      FILE* const fp = std::fopen(fn, "r");
      if(fp==NULL) {
	std::fprintf(stderr, "fopen failed\n");
	throw;
      }

      this->fp = fp;
      this->line = 1;
    }

  public:
    void set_file(FILE* const fp)
    {

      if(this->fp!=NULL) {
	std::fprintf(stderr, "already opening a file\n");
	throw;
      }

      this->fp = fp;
      this->line = 1;

    }

  public:
    void fclose()
    {
      if(fp==NULL) {
	std::fprintf(stderr, "no file opened.\n");
	throw;
      }
      std::fclose(this->fp);
      fp=NULL;
    }

  private:
    int line;
  public:
    int get_line() { return line; }

  private:
    int skip_until_comment_out_multi_line_ends()
    {

      while(true) {
	const int read = fgetc(fp);
	switch(read) {
	case EOF:
	  return EOF;
	case '*': {
	  const int after_asterisk = fgetc(fp);
	  switch(after_asterisk) {
	  case '/':
	    return 0;
	  default:
	    break;
	  }
	  break;
	}
	default:
	  break;
	}

      }

    }

  public:
    int skip_until_return()
    {
      while(true) {
	const int read = fgetc(fp);
	switch(read) {
	case EOF:
	  return EOF;
	case '\n':
	  return '\n';
	default:
	  break;
	}
      }

    }

  private:
    int skip()
    {
      while(true) {
	const int read = fgetc(fp);
	switch(read) {
	case EOF:
	  return EOF;
	  
	case ' ':
	case '\t':
	  break;

	case '\n':
	  ++line;
	  break;

	case '/': {
	  const int after_slash = fgetc(fp);
	  switch(after_slash) {
	  case '*':
	    this->skip_until_comment_out_multi_line_ends();
	    break;
	  case '/':
	    this->skip_until_return();
	    break;
	  default:
	    return ungetc(read, fp);
	  }
	  break;
	}
	default:
	  return ungetc(read, fp);
	}
      }
    }

  public:
    std::string data_string;
    bool data_bool;
    double data_number;

  public:
    template<typename T>
    T get() const;



  public:
    int next()
    {

      if(EOF==skip())
	return EOF;

      double num_read;
      const int num_ret = fscanf(fp, "%lf", &num_read);
      switch(num_ret) {
      case EOF:
	return EOF;
      case 1:
	this->data_number = num_read;
	return '#';
      case 0:
	break;
      }

      const int c = fgetc(fp);

      switch(c) {

      case EOF:
      case '{':
      case '}':
      case '[':
      case ']':
      case ',':
      case ':':
	return c;

	//boolean
      case 't':
	ungetc('t', fp);
	char true_buf[8];
	fscanf(fp, "%s", true_buf);
	if(strcmp(true_buf, "true")!=0) {
	  // todo
	  fprintf(stderr, "[line:%d] unexpected string: %s\n", line, true_buf);
	  throw;
	}
	this->data_bool = true;
	return '?';

      case 'f':
	ungetc('f', fp);
	char false_buf[8];
	fscanf(fp, "%s", false_buf);
	if(strcmp(false_buf, "false")!=0) {
	  // todo
	  fprintf(stderr, ":%d: unexpected string: %s\n", line, false_buf);
	  throw;
	}
	this->data_bool = false;
	return '?';

      case 'n':
	ungetc('n', fp);
	char null_buf[8];
	fscanf(fp, "%s", null_buf);
	if(strcmp(false_buf, "null")!=0) {
	  // todo
	  fprintf(stderr, ":%d: unexpected string: %s\n", line, null_buf);
	  throw;
	}
	return '0';

	// string
      case '"':
	this->data_string.clear();

      json_reader_read_next_string_start:

	const int data_c = fgetc(fp);
	  
	switch(data_c) {
	case EOF:
	  fprintf(stderr, "[line:%d] unexpected EOF\n", line);
	  throw;

	case '\\': {
	  const int escaped = fgetc(fp);
	  
	  switch(escaped) {

	  case 'n':
	    this->data_string.push_back('\n');
	    break;

	  case 'r':
	    this->data_string.push_back('\r');
	    break;

	  case 't':
	    this->data_string.push_back('\t');
	    break;

	  case 'v':
	    this->data_string.push_back('\v');
	    break;

	  case '\\':
	    this->data_string.push_back('\\');
	    break;

	  case '?':
	    this->data_string.push_back('?');
	    break;

	  case '\'':
	    this->data_string.push_back('\'');
	    break;

	  case '"':
	    this->data_string.push_back('"');
	    break;

	  case 'x': {
	    int read_x;
	    const int read_x_ret = fscanf(fp, "%x", &read_x);
	      
	    switch(read_x_ret) {
	    case EOF:
	    case 0:
	      fprintf(stderr, "[line:%d] unexpected string after \\x\n", line);
	      throw;
	    case 1:
	      this->data_string.push_back(read_x);
	      break;
	    }
	    break;
	  }
	    
	  default: {
	    int read_o;
	    const int read_o_ret = fscanf(fp, "%o", &read_o); 
	      
	    switch(read_o_ret) {
	    case EOF:
	    case 0:
	      fprintf(stderr, "[line:%d] unexpected string after \\\n", line);
	      throw;
	    case 1:
	      this->data_string.push_back(read_o);
	      break;
	    }
	    break;
	  } 
	    
	  }
	  goto json_reader_read_next_string_start;
	  
	}

	case '"':
	  goto json_reader_read_next_string_end;

	case '\n':
	  ++line;
	default:
	  this->data_string.push_back(data_c);
	  goto json_reader_read_next_string_start;

	}

      json_reader_read_next_string_end:
	//this->data_string.push_back('\0');
	return 's';
	  
      }

      throw;

    }

  };

  template<>
  std::string reader::get<std::string>() const
  { return this->data_string; }

  template<>
  const char* reader::get<const char*>() const
  { return this->data_string.c_str(); }

  template<>
  bool reader::get<bool>() const
  { return this->data_bool; }

  template<>
  double reader::get<double>() const
  { return this->data_number; }  


  class writer
  {

  public:
    writer()
    {
      this->fp = NULL;
      sprintf(indent_str, " ");
      this->if_compress = false;
    }

  private:
    FILE* fp;

  public:
    void fopen(const char* const fn)
    {
      if(this->fp!=NULL) {
	std::fprintf(stderr, "already opening a file\n");
	throw;
      }

      FILE* const fp = std::fopen(fn, "w");
      if(fp==NULL) {
	std::fprintf(stderr, "fopen failed\n");
	throw;
      }

      this->fp = fp;
      this->line = 1;
      this->level = 0;
    }

  public:
    void set_file(FILE* const fp)
    {

      if(this->fp!=NULL) {
	std::fprintf(stderr, "already opening a file\n");
	throw;
      }

      this->fp = fp;
      this->line = 1;
      this->level = 0;
    }

  public:
    void fclose()
    {
      if(fp==NULL) {
	std::fprintf(stderr, "no file opened.\n");
	throw;
      }
      std::fclose(this->fp);
      fp=NULL;
    }

  private:
    int line;

  public:
    int get_line() { return this->line; }

  private:
    int level;
    char to_write_indent;

  public:
    char indent_str[8];
    char if_compress;

  public:
    void write_indent()
    {
      for(int l=0; l<level; ++l) {
	fprintf(fp, indent_str);
      }
    }


  private:
    void write_indent_if_necessary()
    {
      if(!if_compress&&to_write_indent) {
	write_indent();
	to_write_indent = false;
      }
    }

  public:
    void write_char(const char c)
    {



      switch(c){

      case '\0':
	write_null();
	break;

      case '[':
      case '{':
	write_indent_if_necessary();
	fputc(c, fp);
	++level;

	break;

      case ']':
      case '}':
	--level;
	write_indent_if_necessary();
	fputc(c, fp);
	break;

      case ',':
	write_indent_if_necessary();
	if(if_compress) {
	  fputc(',', fp);
	} else {
	  fprintf(fp, " , ");
	  ++line;
	}
	break;

      case ':':
	write_indent_if_necessary();
	if(if_compress) {
	  fputc(':', fp);
	} else {
	  fprintf(fp, " : ", c);
	}
	break;

      case '\n':
	fputc('\n', fp);
	to_write_indent = true;
	break;

      default:
	fprintf(stderr, "unexpected character %c\n", c);
	throw;

      }
    }



  private:
    void write_escape(const char* const str)
    {

      fputc('"', fp);

      int idx = -1;
    json_writer_escape_start:
      ++idx;

      switch(str[idx]) {

      case '\0':
	goto json_writer_escape_end;

      case '\n':
	fprintf(fp, "\\n");
	goto json_writer_escape_start;

      case '\r':
	fprintf(fp, "\\r");
	goto json_writer_escape_start;

      case '\t':
	fprintf(fp, "\\t");
	goto json_writer_escape_start;

      case '\v':
	fprintf(fp, "\\v");
	goto json_writer_escape_start;

      case '\\':
	fprintf(fp, "\\\\");
	goto json_writer_escape_start;

      case '?':
	fprintf(fp, "\\?");
	goto json_writer_escape_start;

      case '\'':
	fprintf(fp, "\\'");
	goto json_writer_escape_start;

      case '\"':
	fprintf(fp, "\\\"");
	goto json_writer_escape_start;

      default:
	fputc(str[idx], fp);
	goto json_writer_escape_start;

      }


    json_writer_escape_end:
      fputc('"', fp);
	
    }
    
  public:
    void write_string(const char* const str)
    {
      write_indent_if_necessary();

      write_escape(str);

    }

  public:
    void write_number(const double num)
    {
      write_indent_if_necessary();

      // todo:
      fprintf(fp, "%.16g", num);
    }

  public:
    void write_boolean(const bool boolean)
    {
      write_indent_if_necessary();

      // todo:
      if(boolean)
	fprintf(fp, "true");
      else
	fprintf(fp, "false");
    }

  public:
    void write_null()
    {
      write_indent_if_necessary();

      fprintf(fp, "null");
    }

  public:
    template<typename T>
    void write(T value);

    

  };

  template<>
  void writer::write<char>(char c)
  {
    this->write_char(c);
  }

  template<>
  void writer::write<const char*>(const char* str)
  {
    this->write_string(str);
  }

  template<>
  void writer::write<double>(double value)
  {
    this->write_number(value);
  }

  template<>
  void writer::write<bool>(bool value)
  {
    this->write_boolean(value);
  }


}

#endif // JSON_HPP
