package us.achromaticmetaphor.agram;

import java.util.ArrayList;

public class Anagrams implements Generator {

  public native ArrayList<String> generate(String s);

  public ArrayList<String> generate(String s, boolean full) {
    return generate(s);
  }

  public boolean hasLongMode() { return false; }
  public String longLabel() { return ""; }
  public String shortLabel() { return "Generate"; }
  public String inputPrompt() { return "Choose characters"; }

}
